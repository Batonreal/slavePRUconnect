#ifndef MESSAGE_UTILS_H
#define MESSAGE_UTILS_H

#include <vector>
#include <cstdint>
#include "functions.h"

std::vector<uint8_t> createMessage(const Packet& packet, uint64_t& cumulative_impulse);

#endif // MESSAGE_UTILS_H