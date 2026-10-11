# Computer Networks Lab 07 - UDP Sockets

Roll Number: 24625

Port Number: 30625

Service Type: B - Integer List Processor

d = (6 + 2 + 5) mod 3 = 1

Supported Operations:
- SUM
- MAX
- MIN
- AVG

Files:
- udp_server.c
- udp_client.c

Compile:
gcc -Wall -Wextra udp_server.c -o udp_server
gcc -Wall -Wextra udp_client.c -o udp_client

Run Server:
./udp_server

Run Client:
./udp_client 127.0.0.1 24625

Example:
SUM|5|1 2 3 4 5

Output:
15
