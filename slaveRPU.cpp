#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>
#include <iomanip>

#define BUFFER_SIZE 2048

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

        if (n < 16) {
        	std::cerr << "Received insufficient data." << std::endl;
        	continue;
        }

        if ((n - 11) % 56 != 0) {
        	std::cerr << "The number of bytes is incorrect." << std::endl;
        	continue;
        }

        int k = (n - 11) / 56;

        uint32_t cyclic_counter, checksum;
        uint16_t len_info;
        uint8_t message_id;
        uint32_t *phase = new uint32_t[k];
        uint64_t *first_impulse_s = new uint64_t[k];
        uint64_t *first_impulse_ns = new uint64_t[k];
        uint32_t *impulse_ns_2 = new uint32_t[k];
        uint32_t *impulse_ns_3 = new uint32_t[k];
        uint32_t *impulse_ns_4 = new uint32_t[k];
        uint32_t *impulse_ns_5 = new uint32_t[k];
        uint32_t *impulse_ns_6 = new uint32_t[k];
        uint32_t *impulse_ns_7 = new uint32_t[k];
        uint32_t *impulse_ns_8 = new uint32_t[k];
        uint32_t *impulse_ns_9 = new uint32_t[k];
        uint32_t *impulse_ns_10 = new uint32_t[k];

        for (int i = 0; i < n / 2; ++i) {
        	std::swap(buffer[i], buffer[n - 1 - i]);
        }

        size_t current = 0;

        std::cout << "Received data:" << std::endl;

        for (int i = 0; i < k; ++i) {
            memcpy(&impulse_ns_10[i], buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&impulse_ns_9[i], buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&impulse_ns_8[i], buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&impulse_ns_7[i], buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&impulse_ns_6[i], buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&impulse_ns_5[i], buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&impulse_ns_4[i], buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&impulse_ns_3[i], buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&impulse_ns_2[i], buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&first_impulse_ns[i], buffer + current, sizeof(uint64_t)); current += sizeof(uint64_t);
            memcpy(&first_impulse_s[i], buffer + current, sizeof(uint64_t)); current += sizeof(uint64_t);
            memcpy(&phase[i], buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);

            std::cout << "Block " << i << ":" << std::endl;
            std::cout << "  phase[" << i << "] = " << phase[i] << std::endl;
            std::cout << "  first_impulse_s[" << i << "] = " << first_impulse_s[i] << std::endl;
            std::cout << "  first_impulse_ns[" << i << "] = " << first_impulse_ns[i] << std::endl;
            std::cout << "  impulse_ns_2[" << i << "] = " << impulse_ns_2[i] << std::endl;
            std::cout << "  impulse_ns_3[" << i << "] = " << impulse_ns_3[i] << std::endl;
            std::cout << "  impulse_ns_4[" << i << "] = " << impulse_ns_4[i] << std::endl;
            std::cout << "  impulse_ns_5[" << i << "] = " << impulse_ns_5[i] << std::endl;
            std::cout << "  impulse_ns_6[" << i << "] = " << impulse_ns_6[i] << std::endl;
            std::cout << "  impulse_ns_7[" << i << "] = " << impulse_ns_7[i] << std::endl;
            std::cout << "  impulse_ns_8[" << i << "] = " << impulse_ns_8[i] << std::endl;
            std::cout << "  impulse_ns_9[" << i << "] = " << impulse_ns_9[i] << std::endl;
            std::cout << "  impulse_ns_10[" << i << "] = " << impulse_ns_10[i] << std::endl;
        }

        memcpy(&cyclic_counter, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
        memcpy(&checksum, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
        memcpy(&len_info, buffer + current, sizeof(uint16_t)); current += sizeof(uint16_t);
        memcpy(&message_id, buffer + current, sizeof(uint8_t));

        std::cout << "cyclic_counter = " << cyclic_counter << std::endl;
        std::cout << "checksum = " << checksum << std::endl;
        std::cout << "len_info = " << len_info << std::endl;
        std::cout << "message_id = " << static_cast<int>(message_id) << std::endl;
        std::cout << "  Number of bytes received: " << n << std::endl;

        message = "ok";

        if (sendto(sockfd, message.c_str(), message.size(), 0, (struct sockaddr *)&slave_addr, sizeof(slave_addr)) < 0) {
            perror("sendto failed");
            close(sockfd);
            return 1;
        }

        delete[] phase;
        delete[] first_impulse_s;
        delete[] first_impulse_ns;
        delete[] impulse_ns_2;
        delete[] impulse_ns_3;
        delete[] impulse_ns_4;
        delete[] impulse_ns_5;
        delete[] impulse_ns_6;
        delete[] impulse_ns_7;
        delete[] impulse_ns_8;
        delete[] impulse_ns_9;
        delete[] impulse_ns_10;
    }

    close(sockfd);
    return 0;
}
