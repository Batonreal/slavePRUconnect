#include "message_utils.h"

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