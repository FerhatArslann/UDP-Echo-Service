FROM ubuntu:24.04

RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y build-essential gdb wget nano tree valgrind git curl unzip ssh make

WORKDIR /app
COPY . .
RUN make

CMD ["./UDPEchoClient", "-h", "udpechoserver", "-p", "8008", "-m", "Hello from Docker!"]
