# 🛰️ UDP Echo Service – Chunked File Transfer

This project implements a **UDP Echo Server** and **UDP Echo Client** running inside **Docker containers**.
The system supports both message and **file transfer using UDP**, measures transfer time, includes error handling, and passes Valgrind memory checks (no leaks).

---

## ⚙️ Project Overview

The application consists of two main components:

* **UDPEchoServer** – Listens on a UDP port, receives data, and sends back acknowledgments.
* **UDPEchoClient** – Sends either a text message or file in chunks and reports transfer duration.

Project tested in **Ubuntu 24.04 (Docker environment)**.

---

## 📁 Directory Structure

```
UDPEchoService/
├── docker-compose.yml
├── .env
│
├── UDPEchoServer/
│   ├── Dockerfile
│   ├── Makefile
│   └── UDPEchoServer.cpp
│
└── UDPEchoClient/
    ├── Dockerfile
    ├── Makefile
    ├── UDPEchoClient.cpp
    └── Testfiles/
        ├── simple-file.dat
        ├── complex-file-1.dat
        └── complex-file-2.dat
```

---

## 🧩 Build Instructions

### 1️⃣ Build all containers

```bash
docker-compose build
```

This compiles both client and server using their respective **Makefiles**.

### 2️⃣ Run the service

```bash
docker-compose up
```

Both containers start automatically:

* `udpechoserver` listens on port 8008
* `udpechoclient` sends a test message to the server

---

## 💾 Sending Files

To test file transfer, edit `docker-compose.yml`:

```yaml
command: ["./UDPEchoClient", "-h", "udpechoserver", "-p", "${server_port}", "-f", "Testfiles/complex-file-1.dat"]
```

Then rebuild and run:

```bash
docker-compose build udpechoclient
docker-compose up udpechoclient
```

Expected server log:

```
[UDPEchoServer] Listening on port 8008...
[UDPEchoServer] Receiving file: complex-file-1.dat
[UDPEchoServer] File received successfully.
[UDPEchoServer] Sent acknowledgment to client.
```

Expected client output:

```
[UDPEchoClient] Sending file in chunks: complex-file-1.dat
[UDPEchoClient] Transfer complete.
[UDPEchoClient] Transfer time: 0.241 seconds.
```

---

## 🧠 Error Handling

UDP is a connectionless protocol, so the program includes simple handling logic:

* If a packet fails to send, client prints an error.
* If the server does not respond, client times out.
* Missing “FILE_END” message is detected by the server.

Because UDP gives no delivery guarantees, the client and server handle the
gaps themselves: the client reports failed sends and times out if the server
goes quiet, and the server detects a transfer that never sent its FILE_END
marker.

---

## 🧪 Valgrind Memory Test

Run inside the server container:

```bash
docker exec -it udpechoserver bash
valgrind --leak-check=full ./UDPEchoServer -p 8008
```

Expected output:

```
== HEAP SUMMARY:
   in use at exit: 0 bytes in 0 blocks
   total heap usage: 3 allocs, 3 frees, 75,224 bytes allocated
== All heap blocks were freed -- no leaks are possible
== ERROR SUMMARY: 0 errors from 0 contexts
```

✅ **No memory leaks detected**

---

## Tech Stack

- **Language:** C++ (POSIX sockets, `AF_INET` / `SOCK_DGRAM`)
- **Build:** Makefile, g++
- **Containers:** Docker, Docker Compose
- **Testing:** Valgrind (no leaks), Ubuntu 24.04

---

## 🔌 Communication Protocol

* UDP-based communication (`AF_INET`, `SOCK_DGRAM`)
* Client sends:

  * Header: `"FILE:<filename>"`
  * File chunks (1 KB each)
  * `"FILE_END"` message when done
* Server responds with `"ACK"` to confirm receipt
* Client measures total transfer time

This simple request/acknowledgment mechanism ensures reliable completion within UDP limitations.

---

## ⚙️ Environment Variables (.env)

```
server_port=8008
server_testing_port=12000
```

---

## 🧱 Makefile Example

```makefile
CC = g++
CFLAGS = -Wall -O2
TARGET = UDPEchoServer
SRC = $(TARGET).cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
```

---
