#include "functions.h"
#include "message_utils.h"
#include <iostream>
#include <cstring>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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

void crc_check(char* buffer, int n) {
    std::vector<uint8_t> data;
    data.insert(data.end(), buffer, buffer + 3);
    data.insert(data.end(), buffer + 7, buffer + n);

    uint32_t calculated_checksum = calculate_crc32(data.data(), data.size());

    uint32_t received_checksum = (static_cast<uint8_t>(buffer[6]) << 24) |
                                 (static_cast<uint8_t>(buffer[5]) << 16) |
                                 (static_cast<uint8_t>(buffer[4]) << 8) |
                                 static_cast<uint8_t>(buffer[3]);

    if (calculated_checksum != received_checksum) {
        std::cerr << "Checksum verification failed. Data is corrupted." << std::endl;
    } else {
        std::cout << "Checksum verification passed. Data is intact." << std::endl;
    }
}

bool compare_packets(const Packet& p1, const Packet& p2) {
    return p1.packet_id == p2.packet_id;
}

int check_for_duplicates(const std::vector<Packet>& previous_packets, const std::vector<Packet>& current_packets) {
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
            return n;
        }
    }
    return 0;
}

void timeDataRadioimpulse(char* buffer, int n, SpiController& spi, std::vector<Packet>& last_packets) {
    if (n < 16) {
        std::cerr << "Received insufficient data." << std::endl;
        return;
    } else if ((n - 11) % NUM_BYTES_PACKET != 0) {
        std::cerr << "The number of bytes is incorrect." << std::endl;
        return;
    }

    crc_check(buffer, n);

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

    for (int i = k - 1; i > -1; --i) {
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

    int duplicate_count = 0;
    if (!last_packets.empty()) {
        duplicate_count = check_for_duplicates(last_packets, packets);
    }

    last_packets = packets;

    uint64_t cumulative_impulse = 0;
    std::vector<uint8_t> spi_message;

    for (int i = 0; i < duplicate_count; ++i) {
        cumulative_impulse += packets[i].impulse_ns_1;
        cumulative_impulse += packets[i].impulse_ns_2;
        cumulative_impulse += packets[i].impulse_ns_3;
        cumulative_impulse += packets[i].impulse_ns_4;
        cumulative_impulse += packets[i].impulse_ns_5;
        cumulative_impulse += packets[i].impulse_ns_6;
        cumulative_impulse += packets[i].impulse_ns_7;
        cumulative_impulse += packets[i].impulse_ns_8;
        cumulative_impulse += packets[i].impulse_ns_9;
        cumulative_impulse += packets[i].impulse_ns_10;
    }

    for (int i = duplicate_count; i < static_cast<int>(packets.size()); ++i) {
        std::vector<uint8_t> packet_message = createMessage(packets[i], cumulative_impulse);
        spi_message.insert(spi_message.end(), packet_message.begin(), packet_message.end());
    }

    spi.transfer(spi_message);

    memcpy(&cyclic_counter, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
    memcpy(&checksum, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
    memcpy(&len_info, buffer + current, sizeof(uint16_t)); current += sizeof(uint16_t);
    memcpy(&message_id, buffer + current, sizeof(uint8_t));

    std::cout << "cyclic_counter = " << cyclic_counter << std::endl;
    std::cout << "checksum = 0x" << std::setw(8) << std::setfill('0') << std::hex << checksum << std::endl;
    std::cout << std::dec;
    std::cout << "len_info = " << len_info << std::endl;
    std::cout << "message_id = " << static_cast<int>(message_id) << std::endl;
    std::cout << "  Number of bytes received: " << n << std::endl;
}

void dataOnTheAmplitudesRadioimpulse(char* buffer, int n, SpiController& spi) {
    if (n < 16) {
        std::cerr << "Received insufficient data." << std::endl;
        return;
    }

    crc_check(buffer, n);

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
        std::cout << "generator[" << i << "], number turned on = " << static_cast<int>(number_of_generators[i]) << std::endl;
    }

    memcpy(&transfer_dir, buffer + current, sizeof(uint8_t)); current += sizeof(uint8_t);

    if (transfer_dir & 0x80) {
        std::cout << "Direction: 2" << std::endl;
    } else if (transfer_dir & 0x00) {
        std::cout << "Direction: 1" << std::endl;
    } else {
        std::cout << "error direction" << std::endl;
    }

    memcpy(&cyclic_counter, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
    memcpy(&checksum, buffer + current, sizeof(uint32_t)); current += sizeof(uint32_t);
    memcpy(&len_info, buffer + current, sizeof(uint16_t)); current += sizeof(uint16_t);
    memcpy(&message_id, buffer + current, sizeof(uint8_t));

    std::cout << "cyclic_counter = " << cyclic_counter << std::endl;
    std::cout << "checksum = 0x" << std::setw(8) << std::setfill('0') << std::hex << checksum << std::endl;
    std::cout << std::dec;
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

    std::vector<uint8_t> time_interval_as_uint8(32);
    for (int i = 0; i < 32; ++i) {
        time_interval_as_uint8[i] = time_interval[i] ? 1 : 0;
    }
    json_data["time_interval"] = time_interval_as_uint8;

    std::ofstream json_file("data.json");
    json_file << json_data.dump(4);
    json_file.close();
}
