#ifndef URP_URP_H
#define URP_URP_H

#include <cstdint>
#include <functional>
const size_t data_size = 1420;
const int magic_packet = 0xDADADADA;
const int magic_start = 0x1111BE19;
const int magic_end = 0x00EE99DD;
const int magic_exit = 0x00EE88DD;
struct Header
{
    uint32_t magic;
    uint32_t number;
    uint32_t size;
};

struct DataFrame
{
    Header hdr;
    char data[data_size];
};

namespace std
{
    template <>
    struct hash<DataFrame>
    {
        const size_t operator()(const DataFrame &c) const
        {
            return std::hash<int>()(c.hdr.number);
        }
    };
}

bool operator==(const DataFrame &lhs, const DataFrame &rhs)
{
    return lhs.hdr.number == rhs.hdr.number;
}

bool operator<(const DataFrame &lhs, const DataFrame &rhs)
{
    return lhs.hdr.number < rhs.hdr.number;
}

#endif