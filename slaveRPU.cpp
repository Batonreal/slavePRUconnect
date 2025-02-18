#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>
#include <bitset>

#define BUFFER_SIZE 2048

std::string toBinaryString(uint32_t value) {
    return std::bitset<32>(value).to_string();
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <PORT>\n";
        return 1;
    }

    int port = std::stoi(argv[1]);
    const char *server_ip = "192.168.0.1";

    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    std::string message;
    socklen_t addr_len = sizeof(client_addr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return 1;
    }

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
            perror("inet_pton failed");
            close(sockfd);
            return 1;
        }

    std::cout << "Server is listening on port " << port << std::endl;

    while (true) {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) {
            perror("recvfrom failed");
            break;
        }

        if (n < 16) {
                    std::cerr << "Received insufficient data." << std::endl;
                    continue;
        }

        uint32_t bin32, uint32_1, uint32_2, uint32_3;

        for (int i = 0; i < n / 2; ++i) {
                    std::swap(buffer[i], buffer[n - 1 - i]);
                }

        memcpy(&uint32_3, buffer, sizeof(uint32_3));
        memcpy(&uint32_2, buffer + 4, sizeof(uint32_2));
        memcpy(&uint32_1, buffer + 8, sizeof(uint32_1));
        memcpy(&bin32, buffer + 12, sizeof(bin32));

        std::string bin32Str = toBinaryString(bin32);

        std::cout << "Received data:" << std::endl;
        std::cout << "BIN32: " << bin32Str << std::endl;
        std::cout << "UINT32_1: " << uint32_1 << std::endl;
        std::cout << "UINT32_2: " << uint32_2 << std::endl;
        std::cout << "UINT32_3: " << uint32_3 << std::endl;
        std::cout << "Number of bytes received: " << n << std::endl;

        message = "ok";

        if (sendto(sockfd, message.c_str(), message.size(), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("sendto failed");
                close(sockfd);
                return 1;
            }
    }

    close(sockfd);
    return 0;
}
