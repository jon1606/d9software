# Sent for every command received. Topic: /d9/status/ack
uint8 ACCEPTED=0
uint8 BUSY=1

string command  # "clear", "tow" or "stop"
uint8 status
