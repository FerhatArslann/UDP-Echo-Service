// UDPEchoClient.cpp

#include <iostream>
#include <string>
#include <cstring>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include <netdb.h>

struct Config {
    std::string host;
    int port = 8008;
    std::string message;
    std::string filename;
};

Config parseArgs(int argc, char* argv[]) {
    Config cfg;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-h" && i + 1 < argc)
            cfg.host = argv[++i];
        else if (std::string(argv[i]) == "-p" && i + 1 < argc)
            cfg.port = std::stoi(argv[++i]);
        else if (std::string(argv[i]) == "-m" && i + 1 < argc)
            cfg.message = argv[++i];
        else if (std::string(argv[i]) == "-f" && i + 1 < argc)
            cfg.filename = argv[++i];
    }
    return cfg;
}

int main(int argc, char* argv[]) {
    Config cfg = parseArgs(argc, argv);
    if (cfg.host.empty()) {
        std::cerr << "Virhe: isäntänimi (-h) on pakollinen.\n";
        return 1;
    }

    int sockfd;
    struct sockaddr_in serverAddr{};
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Virhe: socketin luonti epäonnistui\n";
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(cfg.port);

    struct hostent* host_entry = gethostbyname(cfg.host.c_str());
    if (host_entry == nullptr) {
        std::cerr << "Virheellinen host: " << cfg.host << "\n";
        return 1;
    }
    memcpy(&serverAddr.sin_addr, host_entry->h_addr_list[0], host_entry->h_length);

    std::cout << "[UDPEchoClient] Yhdistetään palvelimeen "
              << cfg.host << ":" << cfg.port << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    if (!cfg.filename.empty()) {
        FILE* file = fopen(cfg.filename.c_str(), "rb");
        if (!file) {
            std::cerr << "Virhe: tiedostoa ei voitu avata: " << cfg.filename << std::endl;
            return 1;
        }

        std::cout << "[UDPEchoClient] Lähetetään tiedosto paloissa: " << cfg.filename << std::endl;

        std::string header = "FILE:" + cfg.filename;
        sendto(sockfd, header.c_str(), header.size(), MSG_CONFIRM,
               (const struct sockaddr *)&serverAddr, sizeof(serverAddr));
        usleep(10000);

        char buffer[1024];
        size_t bytesRead;
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            ssize_t sent = sendto(sockfd, buffer, bytesRead, MSG_CONFIRM,
                                  (const struct sockaddr *)&serverAddr, sizeof(serverAddr));
            if (sent < 0) {
                std::cerr << "Virhe: lähetys epäonnistui.\n";
                break;
            }
            usleep(10000);
        }

        fclose(file);
        std::cout << "[UDPEchoClient] Tiedoston lähetys valmis." << std::endl;

        const char* eofMsg = "FILE_END";
        sendto(sockfd, eofMsg, strlen(eofMsg), MSG_CONFIRM,
               (const struct sockaddr *)&serverAddr, sizeof(serverAddr));
    } 
    else if (!cfg.message.empty()) {
        sendto(sockfd, cfg.message.c_str(), cfg.message.size(), MSG_CONFIRM,
               (const struct sockaddr *)&serverAddr, sizeof(serverAddr));
        std::cout << "[UDPEchoClient] Lähetettiin viesti: " << cfg.message << std::endl;
    } 
    else {
        std::cerr << "Virhe: ei viestiä eikä tiedostoa määritetty.\n";
        return 1;
    }

    struct timeval timeout{};
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    char buffer[2048];
    socklen_t len = sizeof(serverAddr);
    ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                         (struct sockaddr *)&serverAddr, &len);
    if (n < 0) {
        std::cerr << "[UDPEchoClient] Ei saatu vastausta palvelimelta (timeout tai virhe).\n";
    } else {
        buffer[n] = '\0';
        std::cout << "[UDPEchoClient] Vastaanotettu vastaus: " << buffer << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "[UDPEchoClient] Siirtoaika: " << elapsed.count() << " sekuntia." << std::endl;

    close(sockfd);
    return 0;
}
