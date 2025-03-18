#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <vector>
#include <cstdint>
#include "SpiController.h"

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

bool compare_packets(const Packet& p1, const Packet& p2);
int check_for_duplicates(const std::vector<Packet>& previous_packets, const std::vector<Packet>& current_packets);
std::vector<uint8_t> createMessage(const Packet& packet, uint64_t& cumulative_impulse);
void timeDataRadioimpulse(char* buffer, int n, SpiController& spi, std::vector<Packet>& last_packets);
void dataOnTheAmplitudesRadioimpulse(char* buffer, int n, SpiController& spi);

#endif // FUNCTIONS_H