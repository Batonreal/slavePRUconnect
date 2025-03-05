#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>
#include <iomanip>
#include <vector>

#define BUFFER_SIZE 2048

struct Packet {
    uint32_t phase;
    uint32_t impulse_ns_1;
    uint32_t impulse_ns_2;
    uint32_t impulse_ns_3;
    uint32_t impulse_ns_4;
    uint32_t impulse_ns_5;
    uint32_t impulse_ns_6;
    uint32_t impulse_ns_7;
    uint32_t impulse_ns_8;
    uint32_t impulse_ns_9;
    uint32_t impulse_ns_10;
};

bool compare_packets(const Packet& p1, const Packet& p2) {
    return p1.phase == p2.phase &&
           p1.impulse_ns_1 == p2.impulse_ns_1 &&
           p1.impulse_ns_2 == p2.impulse_ns_2 &&
           p1.impulse_ns_3 == p2.impulse_ns_3 &&
           p1.impulse_ns_4 == p2.impulse_ns_4 &&
           p1.impulse_ns_5 == p2.impulse_ns_5 &&
           p1.impulse_ns_6 == p2.impulse_ns_6 &&
           p1.impulse_ns_7 == p2.impulse_ns_7 &&
           p1.impulse_ns_8 == p2.impulse_ns_8 &&
           p1.impulse_ns_9 == p2.impulse_ns_9 &&
           p1.impulse_ns_10 == p2.impulse_ns_10;
}

void check_for_duplicates(const std::vector<Packet>& previous_packets, const std::vector<Packet>& current_packets) {
    size_t prev_size = previous_packets.size();
    size_t curr_size = current_packets.size();
    size_t max_check = std::min(prev_size, curr_size);

    for (size_t n = 1; n <= max_check; ++n) {
        bool duplicate_found = true;
        for (size_t i = 0; i < n; ++i) {
            if (!compare_packets(previous_packets[prev_size - n + i], current_packets[i])) {
                duplicate_found = false;
                break;
            }
        }
        if (duplicate_found) {
            std::cout << "Duplicate detected: last " << n << " packets of previous message match first " << n << " packets of current message." << std::endl;
        }
    }
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

        if (buffer[0] == static_cast<char>(255)) {
            // Если первый байт равен 255, читаем сообщение как текст
            std::cout << "Received text message: " << std::string(buffer, n) << std::endl;
            continue;
        }

        if (n < 16) {
            std::cerr << "Received insufficient data." << std::endl;
            continue;
        }

        if ((n - 11) % 44 != 0) {
            std::cerr << "The number of bytes is incorrect." << std::endl;
            continue;
        }

        int k = (n - 11) / 44;

        uint32_t cyclic_counter, checksum;
        uint16_t len_info;
        uint8_t message_id;
        std::vector<Packet> packets(k);

        for (int i = 0; i < n / 2; ++i) {
            std::swap(buffer[i], buffer[n - 1 - i]);
        }

        size_t current = 0;

        std::cout << "Received data:" << std::endl;

        for (int i = k; i > 0; --i) {
            memcpy(&packets[i].impulse_ns_10, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&packets[i].impulse_ns_9, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&packets[i].impulse_ns_8, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&packets[i].impulse_ns_7, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&packets[i].impulse_ns_6, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&packets[i].impulse_ns_5, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&packets[i].impulse_ns_4, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&packets[i].impulse_ns_3, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&packets[i].impulse_ns_2, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&packets[i].impulse_ns_1, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
            memcpy(&packets[i].phase, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);

            std::cout << "Block " << i << ":" << std::endl;
            std::cout << "  phase[" << i << "] = " << packets[i].phase << std::endl;
            std::cout << "  impulse_ns_1[" << i << "] = " << packets[i].impulse_ns_1 << std::endl;
            std::cout << "  impulse_ns_2[" << i << "] = " << packets[i].impulse_ns_2 << std::endl;
            std::cout << "  impulse_ns_3[" << i << "] = " << packets[i].impulse_ns_3 << std::endl;
            std::cout << "  impulse_ns_4[" << i << "] = " << packets[i].impulse_ns_4 << std::endl;
            std::cout << "  impulse_ns_5[" << i << "] = " << packets[i].impulse_ns_5 << std::endl;
            std::cout << "  impulse_ns_6[" << i << "] = " << packets[i].impulse_ns_6 << std::endl;
            std::cout << "  impulse_ns_7[" << i << "] = " << packets[i].impulse_ns_7 << std::endl;
            std::cout << "  impulse_ns_8[" << i << "] = " << packets[i].impulse_ns_8 << std::endl;
            std::cout << "  impulse_ns_9[" << i << "] = " << packets[i].impulse_ns_9 << std::endl;
            std::cout << "  impulse_ns_10[" << i << "] = " << packets[i].impulse_ns_10 << std::endl;
        }

        if (!last_packets.empty()) {
            check_for_duplicates(last_packets, packets);
        }

        last_packets = packets;

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

    }

    close(sockfd);
    return 0;
}
