# UDP-emulator

Client server comunication that passes through a UDP emulator

## Design

### Emulator:

Args:
p = probability to drop packet
d = delay before sending packets
q = max que length
A = port A
B = port B
C = port C

Main thread:
Port A will have a socket binded and wil andle all sending and recv
Port A will recieve a packet and check the source
if rand() returns a number less then p the packet is dropped
if it is above p we continue
if the packet is from B then it will be qued to C on condition
if the packet is from C then it will be qued to B on condition
Condition: if rand() is less then 100 - p and ListCount < q

Send thread:
every d seconds it will check a Que
if there is something to be sent it will send it
if not it moves to the next Que 
loop

### Server

Args:
port = port to binded on
cport = client port

Main thread:
Server will bind a socket and wait for inital packet
After initial packet is recievd the sender thread is created for that client
main thread will continue waiting for packets and check them
if ACK it sets the flag to 0
if NACK1 it sets the flag to 1
if NACK2 it sets the flag to 2
flags are used to tell the send thread what to do

Send thread:
in a loop
depending on flag it will carry out an operation
flag = 3: send the current window
flag = 1: resend window index 1
flag = 2: resend window index 2
flag = 0: slide the window
flag = 5: all frames have been sent, kill thread


### Client

Args:
port = port to bind on
servport = server port
delay = used for flow control, how long to wait before resending NACK or ACK

socket will be binded to port and wait for packets
timeout is set on socket
client will keep sending inital packet until it starts recv frames
while client is waiting for frames
on each time out it will send a NACK based on what the position of its window is
client will send ACK onces both frames have been recv and keep resending until next frame has been recv

Protocol
client will send numbers as packets to indicate state
0 = ACK
1 = NACK for window index 1

## Output

mam024@tux5:~/Documents/434/A2/cmpt434-group07/A2> ./server 30025 │^C
30024                                                             │mam024@tux5:~/Documents/434/A2/cmpt434-group07/A2> ./client 30027 30024
sending frames to client                                          │ 15
Frame 1 sent: hey                                                 │Frame 0 not recv send NACK for window index 1
Frame 2 sent: how                                                 │^C
ACK recv for current window state                                 │mam024@tux5:~/Documents/434/A2/cmpt434-group07/A2> ./client 30027 30024
Slidig window                                                     │ 3
Frame 3 sent: are                                                 │Frame 0 not recv send NACK for window index 1
Frame 4 sent: you                                                 │Frame recv: hey
ACK recv for current window state                                 │Frame recv: how
Slidig window                                                     │Both Frames recv sending ACK
Frame 5 sent: doing                                               │Frame recv: are
Frame 6 sent: today                                               │Frame recv: you
ACK recv for current window state                                 │Both Frames recv sending ACK
Slidig window                                                     │Frame recv: doing
Frame 7 sent: do                                                  │Frame recv: today
Frame 8 sent: you                                                 │Both Frames recv sending ACK
ACK recv for current window state                                 │Frame recv: do
Slidig window                                                     │Frame recv: you
Frame 9 sent: need                                                │Both Frames recv sending ACK
Frame 10 sent: anything                                           │Frame recv: need
ACK recv for current window state                                 │Frame recv: anything
All frames sent                                                   │Both Frames recv sending ACK
^C                                                                │All Frames recieved exitingmam024@tux5:~/Documents/434/A2/cmpt434-group
mam024@tux5:~/Documents/434/A2/cmpt434-group07/A2>                │mam024@tux5:~/Documents/434/A2/cmpt434-group07/A2>
──────────────────────────────────────────────────────────────────┴───────────────────────────────────────────────────────────────────────
Port A recieved a packet from Port: 30025
Packet added to Que to C
Port A recieved a packet from Port: 30025
Packet added to Que to C
Ready to send Packet to C
Packet sent to C
Ready to send Packet to C
Packet sent to C
Port A recieved a packet from Port: 30027
Packet added to Que to B
Ready to send Packet to B
Packet sent to B
Port A recieved a packet from Port: 30025
Packet added to Que to C
Port A recieved a packet from Port: 30025
Packet added to Que to C
Ready to send Packet to C
Packet sent to C
Ready to send Packet to C
Packet sent to C
Port A recieved a packet from Port: 30027
Packet added to Que to B
Ready to send Packet to B
Packet sent to B

2 = NACK for window index 2
