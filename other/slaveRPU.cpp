#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>
#include <iomanip>
#include <vector>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <sstream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

#define BUFFER_SIZE 2048
#define NUM_BYTES_PACKET 45

struct Packet {
    uint8_t packet_id;
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

class SpiController {
public:
    SpiController(const std::string& device) : devicePath(device) {
        openDevice();
    }

    ~SpiController() {
        closeDevice();
    }

    void setSpeed(uint32_t speedHz) {
        if (ioctl(fileDescriptor, SPI_IOC_WR_MAX_SPEED_HZ, &speedHz) == -1) {
            throw std::runtime_error("Ошибка при установке скорости SPI");
        }
    }

    std::vector<uint8_t> transfer(const std::vector<uint8_t>& txData) {
        std::vector<uint8_t> rxData(txData.size());

        spi_ioc_transfer transfer;
        memset(&transfer, 0, sizeof(transfer)); // Инициализация нулями

        transfer.tx_buf = (unsigned long)txData.data();
        transfer.rx_buf = (unsigned long)rxData.data();
        transfer.len = txData.size();
        transfer.speed_hz = 0; // Используем установленную скорость

        if (ioctl(fileDescriptor, SPI_IOC_MESSAGE(1), &transfer) < 0) {
            throw std::runtime_error("Ошибка при передаче SPI");
        }

        return rxData;
    }

private:
    void openDevice() {
        fileDescriptor = open(devicePath.c_str(), O_RDWR);
        if (fileDescriptor < 0) {
            std::stringstream ss;
            ss << "Ошибка открытия устройства SPI: " << devicePath;
            throw std::runtime_error(ss.str());
        }
    }

    void closeDevice() {
        if (fileDescriptor != -1) {
            close(fileDescriptor);
        }
    }

    std::string devicePath;
    int fileDescriptor = -1;
};

bool compare_packets(const Packet& p1, const Packet& p2) {
    return p1.packet_id == p2.packet_id;
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

std::vector<uint8_t> createMessage(const Packet& packet, uint64_t& cumulative_impulse) {
    std::vector<uint8_t> message;
    message.push_back((packet.phase >> 24) & 0xFF);
    message.push_back((packet.phase >> 16) & 0xFF);
    message.push_back((packet.phase >> 8) & 0xFF);
    message.push_back(packet.phase & 0xFF);

    cumulative_impulse += packet.impulse_ns_1;
    uint32_t impulse_divided = (cumulative_impulse / 100) % 100000000;
    message.push_back((impulse_divided >> 24) & 0xFF);
    message.push_back((impulse_divided >> 16) & 0xFF);
    message.push_back((impulse_divided >> 8) & 0xFF);
    message.push_back(impulse_divided & 0xFF);

    cumulative_impulse += packet.impulse_ns_2;
    impulse_divided = (cumulative_impulse / 100) % 100000000;
    message.push_back((impulse_divided >> 24) & 0xFF);
    message.push_back((impulse_divided >> 16) & 0xFF);
    message.push_back((impulse_divided >> 8) & 0xFF);
    message.push_back(impulse_divided & 0xFF);

    cumulative_impulse += packet.impulse_ns_3;
    impulse_divided = (cumulative_impulse / 100) % 100000000;
    message.push_back((impulse_divided >> 24) & 0xFF);
    message.push_back((impulse_divided >> 16) & 0xFF);
    message.push_back((impulse_divided >> 8) & 0xFF);
    message.push_back(impulse_divided & 0xFF);

    cumulative_impulse += packet.impulse_ns_4;
    impulse_divided = (cumulative_impulse / 100) % 100000000;
    message.push_back((impulse_divided >> 24) & 0xFF);
    message.push_back((impulse_divided >> 16) & 0xFF);
    message.push_back((impulse_divided >> 8) & 0xFF);
    message.push_back(impulse_divided & 0xFF);

    cumulative_impulse += packet.impulse_ns_5;
    impulse_divided = (cumulative_impulse / 100) % 100000000;
    message.push_back((impulse_divided >> 24) & 0xFF);
    message.push_back((impulse_divided >> 16) & 0xFF);
    message.push_back((impulse_divided >> 8) & 0xFF);
    message.push_back(impulse_divided & 0xFF);

    cumulative_impulse += packet.impulse_ns_6;
    impulse_divided = (cumulative_impulse / 100) % 100000000;
    message.push_back((impulse_divided >> 24) & 0xFF);
    message.push_back((impulse_divided >> 16) & 0xFF);
    message.push_back((impulse_divided >> 8) & 0xFF);
    message.push_back(impulse_divided & 0xFF);

    cumulative_impulse += packet.impulse_ns_7;
    impulse_divided = (cumulative_impulse / 100) % 100000000;
    message.push_back((impulse_divided >> 24) & 0xFF);
    message.push_back((impulse_divided >> 16) & 0xFF);
    message.push_back((impulse_divided >> 8) & 0xFF);
    message.push_back(impulse_divided & 0xFF);

    cumulative_impulse += packet.impulse_ns_8;
    impulse_divided = (cumulative_impulse / 100) % 100000000;
    message.push_back((impulse_divided >> 24) & 0xFF);
    message.push_back((impulse_divided >> 16) & 0xFF);
    message.push_back((impulse_divided >> 8) & 0xFF);
    message.push_back(impulse_divided & 0xFF);

    cumulative_impulse += packet.impulse_ns_9;
    impulse_divided = (cumulative_impulse / 100) % 100000000;
    message.push_back((impulse_divided >> 24) & 0xFF);
    message.push_back((impulse_divided >> 16) & 0xFF);
    message.push_back((impulse_divided >> 8) & 0xFF);
    message.push_back(impulse_divided & 0xFF);

    cumulative_impulse += packet.impulse_ns_10;
    impulse_divided = (cumulative_impulse / 100) % 100000000;
    message.push_back((impulse_divided >> 24) & 0xFF);
    message.push_back((impulse_divided >> 16) & 0xFF);
    message.push_back((impulse_divided >> 8) & 0xFF);
    message.push_back(impulse_divided & 0xFF);

    return message;
}

void timeDataRadioimpulse(char* buffer, int n, SpiController& spi, std::vector<Packet>& last_packets) {
    if (n < 16) {
        std::cerr << "Received insufficient data." << std::endl;
        return;
    }

    if ((n - 11) % NUM_BYTES_PACKET != 0) {
        std::cerr << "The number of bytes is incorrect." << std::endl;
        return;
    }

    int k = (n - 11) / NUM_BYTES_PACKET;

    uint32_t cyclic_counter, checksum;
    uint16_t len_info;
    uint8_t message_id;
    std::vector<Packet> packets(k);

    for (int i = 0; i < n / 2; ++i) {
        std::swap(buffer[i], buffer[n - 1 - i]);
    }

    size_t current = 0;

    std::cout << "Received data:" << std::endl;

    for (int i = k-1; i > -1; --i) {
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
        memcpy(&packets[i].packet_id, buffer + current, sizeof(uint8_t)); current += sizeof(uint8_t);

        std::cout << "Block " << i << ":" << std::endl;
        std::cout << "  packet_id[" << i << "] = " << static_cast<int>(packets[i].packet_id) << std::endl;
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

    // Выводим информацию о полученных пакетах SPI
    uint64_t cumulative_impulse = 0;
    std::vector<uint8_t> spi_message;
    for (const auto& packet : packets) {
        std::vector<uint8_t> packet_message = createMessage(packet, cumulative_impulse);
        spi_message.insert(spi_message.end(), packet_message.begin(), packet_message.end());
    }
    spi.transfer(spi_message);

    memcpy(&cyclic_counter, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
    memcpy(&checksum, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
    memcpy(&len_info, buffer + current, sizeof(uint16_t)); current += sizeof(uint16_t);
    memcpy(&message_id, buffer + current, sizeof(uint8_t));

    std::cout << "cyclic_counter = " << cyclic_counter << std::endl;
    std::cout << "checksum = 0x" << std::setw(8) << std::setfill('0') << std::hex << checksum << std::endl;
    std::cout << "len_info = " << len_info << std::endl;
    std::cout << "message_id = " << static_cast<int>(message_id) << std::endl;
    std::cout << "  Number of bytes received: " << n << std::endl;
}

void dataOnTheAmplitudesRadioimpulse(char* buffer, int n, SpiController& spi) {
    if (n < 16) {
        std::cerr << "Received insufficient data." << std::endl;
        return;
    }

    uint32_t cyclic_counter, checksum;
    uint16_t len_info;
    uint8_t message_id;
    uint32_t swithed_on;
    bool time_interval[32];
    uint8_t generator[16];
    uint8_t number_of_generators[32];
    uint8_t transfer_dir;

    for (int i = 0; i < n / 2; ++i) {
        std::swap(buffer[i], buffer[n - 1 - i]);
    }

    size_t current = 0;

    std::cout << "Received data:" << std::endl;

    memcpy(&swithed_on, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);

    for (int i = 0; i < 32; ++i) {
        time_interval[i] = (swithed_on & (1 << (31 - i))) != 0;
    }

    std::cout << "time_interval = ";
    for (int i = 0; i < 32; ++i) {
        std::cout << time_interval[i];
        if ((i + 1) % 4 == 0) std::cout << ' ';
    }
    std::cout << std::endl;

    for (int i = 0; i < 16; ++i) {
        memcpy(&generator[i], buffer + current, sizeof(uint8_t)); current += sizeof(uint8_t);
    }

    for (int i = 0; i < 16; ++i) {
        uint8_t high_nibble = (generator[i] >> 4) & 0x0F;
        uint8_t low_nibble = generator[i] & 0x0F;

        number_of_generators[2 * i] = high_nibble + 1;
        number_of_generators[2 * i + 1] = low_nibble + 1;
    }

    for (int i = 0; i < 32; ++i) {
        std::cout << "generator[" << i << "], the number of generators turned on per half-wave = " << static_cast<int>(number_of_generators[i]) << std::endl;
    }

    memcpy(&transfer_dir, buffer + current, sizeof(uint8_t)); current += sizeof(uint8_t);

    // Определяем направление по первому (старшему) биту transfer_dir
    if (transfer_dir & 0x80) {
        std::cout << "Direction: 2" << std::endl;
    } else {
        std::cout << "Direction: 1" << std::endl;
    }

    memcpy(&cyclic_counter, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
    memcpy(&checksum, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
    memcpy(&len_info, buffer + current, sizeof(uint16_t)); current += sizeof(uint16_t);
    memcpy(&message_id, buffer + current, sizeof(uint8_t));

    std::cout << "cyclic_counter = " << cyclic_counter << std::endl;
    std::cout << "checksum = 0x" << std::setw(8) << std::setfill('0') << std::hex << checksum << std::endl;
    std::cout << "len_info = " << len_info << std::endl;
    std::cout << "message_id = " << static_cast<int>(message_id) << std::endl;

    std::ifstream infile("data.json");
    if (!infile.good()) {
        std::ofstream json_file("data.json");
        json_file.close();
    }

    json json_data;
    json_data["transfer_dir"] = transfer_dir;
    json_data["number_of_generators"] = std::vector<uint8_t>(number_of_generators, number_of_generators + 32);
    json_data["time_interval"] = std::vector<bool>(time_interval, time_interval + 32);

    std::ofstream json_file("data.json");
    json_file << json_data.dump(4);
    json_file.close();
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
