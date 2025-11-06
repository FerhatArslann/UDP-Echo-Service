// UDPEchoServer.cpp

#include <iostream>
#include <cstring>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstdio>

#define DEFAULT_SERVER_PORT 8008
#define BUFFER_SIZE 2048

struct Config {
    int port = DEFAULT_SERVER_PORT;
    int delay_in_secs = 0;
};

static int sockid = -1;
bool running = true;

void INTHandler(int sig) {
    std::cout << "\n[UDPEchoServer] Sammutuspyyntö vastaanotettu (Ctrl+C)" << std::endl;
    running = false;
    if (sockid != -1) close(sockid);
}

void GetCmdLineOptions(int argc, char *argv[], Config *cfg) {
    int opt;
    while ((opt = getopt(argc, argv, "p:d:")) != -1) {
        switch (opt) {
            case 'p':
                cfg->port = atoi(optarg);
                break;
            case 'd':
                cfg->delay_in_secs = atoi(optarg);
                break;
            default:
                std::cerr << "Käyttö: ./UDPEchoServer [-p portti] [-d viive]\n";
                exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    Config cfg;
    GetCmdLineOptions(argc, argv, &cfg);

    struct sockaddr_in serverAddr, clientAddr;
    socklen_t len;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE + 50];
    int msg_count = 0;

    std::signal(SIGINT, INTHandler);

    if ((sockid = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socketin luonti epäonnistui");
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    memset(&clientAddr, 0, sizeof(clientAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(cfg.port);

    if (bind(sockid, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind epäonnistui");
        close(sockid);
        return 1;
    }

    std::cout << "[UDPEchoServer] Kuuntelee porttia " << cfg.port << "...\n";

    len = sizeof(clientAddr);
    FILE* fileOut = nullptr;
    std::string currentFileName;

    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        int n = recvfrom(sockid, buffer, BUFFER_SIZE, MSG_WAITALL,
                         (struct sockaddr *)&clientAddr, &len);

        if (n <= 0) {
            if (running) perror("recvfrom epäonnistui");
            continue;
        }

        buffer[n] = '\0';
        msg_count++;

        if (strncmp(buffer, "FILE:", 5) == 0) {
            currentFileName = std::string(buffer + 5);
            if (fileOut) fclose(fileOut);
            fileOut = fopen(currentFileName.c_str(), "wb");
            if (!fileOut) {
                std::cerr << "[UDPEchoServer] Virhe: ei voitu avata tiedostoa kirjoitusta varten: "
                          << currentFileName << std::endl;
                continue;
            }
            std::cout << "[UDPEchoServer] Aloitetaan tiedoston vastaanotto: "
                      << currentFileName << std::endl;
            continue;
        }

        if (strncmp(buffer, "FILE_END", 8) == 0) {
            if (fileOut) {
                fclose(fileOut);
                fileOut = nullptr;
                std::cout << "[UDPEchoServer] Tiedosto vastaanotettu: " 
                          << currentFileName << std::endl;
            }
            snprintf(response, sizeof(response), "ACK: tiedosto %s vastaanotettu", currentFileName.c_str());
            sendto(sockid, response, strlen(response), MSG_CONFIRM,
                   (const struct sockaddr *)&clientAddr, len);
            std::cout << "[UDPEchoServer] Lähetetty kuittaus tiedoston valmistumisesta.\n";
            continue;
        }

        if (!fileOut && strncmp(buffer, "FILE:", 5) != 0) {
            std::cout << "[UDPEchoServer] Viesti (" << msg_count << ") vastaanotettu osoitteesta "
                      << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port)
                      << " → " << buffer << std::endl;

            if (cfg.delay_in_secs > 0)
                sleep(cfg.delay_in_secs);

            snprintf(response, sizeof(response), "ACK: [%d] %s", msg_count, buffer);
            sendto(sockid, response, strlen(response), MSG_CONFIRM,
                   (const struct sockaddr *)&clientAddr, len);
            std::cout << "[UDPEchoServer] Lähetetty kuittaus.\n";
            continue;
        }

        if (fileOut) {
            fwrite(buffer, 1, n, fileOut);
        }
    }

    if (fileOut) fclose(fileOut);
    std::cout << "[UDPEchoServer] Sammutetaan palvelin...\n";
    close(sockid);
    return 0;
}
