# this is a python script for the raspberry to run to assist with routing traffic - this is especially useful when traffic can be heavy.

import serial
import struct
import time
import threading

# mirrored lora packet structure
LORA_PACKET_FORMAT = '<BBBBBB64s'
LORA_PACKET_SIZE = struct.calcsize(LORA_PACKET_FORMAT)

class LoRaPacket:
    def __init__(self, senderID=0, targetID=0, messageID=0, hopCount=0, maxHops=3, lastRepeater=0, payload=''):
        self.senderID = senderID
        self.targetID = targetID
        self.messageID = messageID
        self.hopCount = hopCount
        self.maxHops = maxHops
        self.lastRepeater = lastRepeater
        # ensure payload is 64 bytes, padded with null bytes
        self.payload = payload.encode('utf-8')[:64].ljust(64, b'\x00')

    def to_bytes(self):
        return struct.pack(LORA_PACKET_FORMAT,
                           self.senderID,
                           self.targetID,
                           self.messageID,
                           self.hopCount,
                           self.maxHops,
                           self.lastRepeater,
                           self.payload)

    @staticmethod
    def from_bytes(data):
        unpacked = struct.unpack(LORA_PACKET_FORMAT, data)
        packet = LoRaPacket(senderID=unpacked[0],
                            targetID=unpacked[1],
                            messageID=unpacked[2],
                            hopCount=unpacked[3],
                            maxHops=unpacked[4],
                            lastRepeater=unpacked[5],
                            # decode and strip null bytes
                            payload=unpacked[6].decode('utf-8').strip('\x00'))
        return packet

    def __str__(self):
        return (f"------ LoRa Packet ------\n"
                f"From: {self.senderID}\n"
                f"To: {self.targetID}\n"
                f"MsgID: {self.messageID}\n"
                f"Hop: {self.hopCount}\n"
                f"MaxHop: {self.maxHops}\n"
                f"Via: {self.lastRepeater}\n"
                f"Payload: {self.payload.decode('utf-8').strip('\x00')}\n"
                f"-------------------------")

# gateway config
GATEWAY_NODE_ID = 3  # id of gateway

SERIAL_PORT = '/dev/ttyACM0'  # IMPORTANT: change to your own serial port
BAUD_RATE = 9600

# helps to prevent the same message going back and forth to the same gateway
last_message_ids = {}
target_node_to_respond = None
message_counter = 0

def init_serial():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Serial port {SERIAL_PORT} opened successfully.")
        return ser
    except serial.SerialException as e:
        print(f"Error opening serial port {SERIAL_PORT}: {e}")
        return None

def send_response_packet(ser, target_id):
    global message_counter
    print(f"DEBUG: Preparing to send response to {target_id}")
    response_payload = f"ACK from G{GATEWAY_NODE_ID} Msg {message_counter}"
    response_packet = LoRaPacket(senderID=GATEWAY_NODE_ID, targetID=target_id,
                                 messageID=message_counter, payload=response_payload)
    message_counter += 1

    try:
        ser.write(response_packet.to_bytes())
        print("\nSent response packet:")
        print(response_packet)
    except Exception as e:
        print(f"Error sending response packet: {e}")

def main():
    global target_node_to_respond
    ser = init_serial()
    if not ser:
        return

    print(f"LoRa Mesh Gateway (Node ID: {GATEWAY_NODE_ID}) started.")
    print("Waiting for packets...")

    # clear any residual data in the buffer before starting
    ser.reset_input_buffer()

    while True:
        if ser.in_waiting >= LORA_PACKET_SIZE:
            try:
                data = ser.read(LORA_PACKET_SIZE)

                # check if the packet looks reasonable
                unpacked = struct.unpack(LORA_PACKET_FORMAT, data)
                # unpacked[4] is maxHops, unpacked[1] is targetID
                if unpacked[4] > 10 or unpacked[1] > 250: # adjust based on network
                    print(f"DEBUG: Potentially invalid packet structure: {unpacked}")
                    ser.reset_input_buffer()
                    continue

                packet = LoRaPacket.from_bytes(data)

                print("\nReceived packet:")
                print(packet)

                # routing
                if packet.targetID == GATEWAY_NODE_ID or packet.targetID == 0:
                    print(f"Packet is for this gateway or broadcast. Processing...")
                    # update last message ID for the sender
                    if packet.senderID not in last_message_ids or \
                       packet.messageID > last_message_ids[packet.senderID]:
                        last_message_ids[packet.senderID] = packet.messageID
                        target_node_to_respond = packet.senderID # Set the sender as the target for response
                        print(f"DEBUG: Set target_node_to_respond to {target_node_to_respond}")
                        # automatically send a response to broadcast/gateway-addressed messages
                        send_response_packet(ser, target_node_to_respond)
                    else:
                        print(f"DEBUG: Ignoring duplicate or old message ID from {packet.senderID}. Last seen: {last_message_ids.get(packet.senderID)}")

                elif packet.hopCount < packet.maxHops:
                    print(f"Packet is for node {packet.targetID}. Forwarding...")
                    packet.hopCount += 1
                    packet.lastRepeater = GATEWAY_NODE_ID
                    ser.write(packet.to_bytes()) # transmit the modified packet
                    print("Packet forwarded.")
                else:
                    print(f"Packet reached maximum hops ({packet.maxHops}). Dropping.")

            except Exception as e:
                print(f"Error processing packet: {e}")
                ser.reset_input_buffer()

if __name__ == "__main__":
    main()
