FROM ubuntu:24.04

# Asennetaan kehitystyökalut
RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y build-essential gdb wget nano tree valgrind git curl unzip ssh make

WORKDIR /app
COPY . .
RUN make

CMD ["./UDPEchoServer", "-p", "8008"]
