#include "memory_utility.h"

void pack_uint32(std::vector<char>& buffer, uint32_t value) {
    buffer.push_back(static_cast<char>((value >> 24) & 0xFF));
    buffer.push_back(static_cast<char>((value >> 16) & 0xFF));
    buffer.push_back(static_cast<char>((value >> 8)  & 0xFF));
    buffer.push_back(static_cast<char>(value         & 0xFF));
}

void pack_uint16(std::vector<char>& buffer, uint16_t value) {
    buffer.push_back(static_cast<char>((value >> 8) & 0xFF));
    buffer.push_back(static_cast<char>(value        & 0xFF));
}

void pack_float(std::vector<char>& buffer, float value) {
    uint32_t int_representation = std::bit_cast<uint32_t>(value);
    pack_uint32(buffer, int_representation);
}