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

uint32_t calculate_crc32(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    uint32_t polynomial = 0xEDB88320;

    for (size_t i = 0; i < length; ++i) {
        uint8_t byte = data[i];
        crc ^= byte;
        for (int j = 0; j < 8; ++j) {
            if (crc & 1) {
                crc = (crc >> 1) ^ polynomial;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc ^ 0xFFFFFFFF;
}

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

        // Извлекаем контрольную сумму (4 байта в формате little-endian)
        uint32_t received_checksum = (static_cast<uint8_t>(buffer[6]) << 24) |
                                     (static_cast<uint8_t>(buffer[5]) << 16) |
                                     (static_cast<uint8_t>(buffer[4]) << 8) |
                                     static_cast<uint8_t>(buffer[3]);

        // Вычисляем контрольную сумму для первых 3 байт и оставшейся части сообщения
        uint32_t calculated_checksum = calculate_crc32(reinterpret_cast<uint8_t*>(buffer), 3);
        calculated_checksum = calculate_crc32(reinterpret_cast<uint8_t*>(buffer + 7), n - 7);

        // Проверяем контрольную сумму
        if (calculated_checksum != received_checksum) {
            std::cerr << "Checksum verification failed. Data is corrupted." << std::endl;
            continue;
        }

        std::cout << "Checksum verification passed. Data is intact." << std::endl;

        // Обрабатываем сообщение в зависимости от типа
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