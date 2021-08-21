#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>
#include <string>
#include <assert.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

void random_fill(u8 *data, int len)
{
    for (int i = 0; i < len; i++)
    {
        data[i] = rand();
    }
}

void from_hex(const std::string &input, u8 *output)
{
    assert((input.size() % 2) == 0);
    for (int i = 0; i < input.size(); i += 2)
    {
        output[i / 2] = std::stoi(input.substr(i, 2), 0, 16);
    }
}

std::string to_hex(u8 *input, int len)
{
    std::string str("");
    std::string str2("0123456789abcdef");
    for (int i = 0; i < len; i++)
    {
        if (i > 0 && i % 32 == 0)
            str.append(1, '\n');
        str.append(1, str2.at(input[i] >> 4));
        str.append(1, str2.at(input[i] & 0x0f));
    }
    return str;
}

void assert_eq(const u8 *in1, const u8 *in2, int len)
{
    for (int i = 0; i < len; i++)
        assert(in1[i] == in2[i]);
}

#endif