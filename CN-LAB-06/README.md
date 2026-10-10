# Computer Networks Lab-06

## Experiment
Multi-Client File Query Server using fork()

## Objective
To implement a TCP client-server application in C where
multiple clients can query student records concurrently.

## Files
- server.c: TCP server that creates a child process using fork()
- client.c: Reads local data.txt and sends a Student ID and records
- data.txt: Sample student records

## Requirements
- Kali Linux or another Linux distribution
- GCC compiler
- Git

## Compilation
```bash
gcc -Wall -Wextra server.c -o server
gcc -Wall -Wextra client.c -o client
```

## Execution
Terminal 1:
```bash
./server
```

Terminal 2:
```bash
./client
```

## Sample Records
```text
101 Amit CSE
102 Rahul ECE
103 Priya CSE
104 Neha EE
105 Rohit ME
```

## Sample Output
```text
Enter Student ID to search: 103
Server Response: Record Found: 103 Priya CSE
```

For an unknown ID:
```text
Server Response: Record Not Found
```

## Concepts Used
- TCP sockets
- Client-server communication
- Process creation using fork()
- Concurrent client handling
- File handling in C
-
