#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>
#include <vector>
#include "SpiController.h"
#include "functions.h"

#define BUFFER_SIZE 2048
#define NUM_BYTES_PACKET 45

int main(int argc, char *argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <PORT_master> <PORT_slave>\n";
        return 1;
    }

    int port_master = std::stoi(argv[1]);
    int port_slave = std::stoi(argv[2]);
    const char *server_ip = "192.168.0.1";

    int sockfd;
    struct sockaddr_in server_addr, client_addr, slave_addr;
    char buffer[BUFFER_SIZE];
    std::string message;
    socklen_t addr_len = sizeof(client_addr);
    std::vector<Packet> last_packets;

    SpiController spi("/dev/spidev1.0");
    spi.setSpeed(1000000); // Set SPI speed to 1 MHz

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    memset(&slave_addr, 0, sizeof(slave_addr));

    slave_addr.sin_family = AF_INET;
    slave_addr.sin_port = htons(port_slave);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_master);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return 1;
    }

    if (inet_pton(AF_INET, server_ip, &slave_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sockfd);
        return 1;
    }

    std::cout << "Server is listening on port " << port_master << std::endl;

    while (true) {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) {
            perror("recvfrom failed");
            break;
        }

        std::cout << "  Number of bytes received: " << n << std::endl;

        switch (buffer[0]) {
            case static_cast<char>(255):
                // Если первый байт равен 255, читаем сообщение как текст, исключая первый байт
                std::cout << "Received text message: " << std::string(buffer + 1, n - 1) << std::endl;
                continue;

            case static_cast<char>(3):
                timeDataRadioimpulse(buffer, n, spi, last_packets);
                break;

            case static_cast<char>(5):
                dataOnTheAmplitudesRadioimpulse(buffer, n, spi);
                break;

            default:
                std::cerr << "Unknown message type." << std::endl;
                continue;
        }

        // Отправляем подтверждение о получении пакетов обратно по протоколу UDP
        message = "ok";

        if (sendto(sockfd, message.c_str(), message.size(), 0, (struct sockaddr *)&slave_addr, sizeof(slave_addr)) < 0) {
            perror("sendto failed");
            close(sockfd);
            return 1;
        }
    }

    close(sockfd);
    return 0;
}