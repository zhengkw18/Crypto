#include "utils.h"
#include "time.h"
#include "stdio.h"
#include <openssl/rc4.h>

// g++ rc4.cpp -o rc4 -lssl -lcrypto -O2

u8 s[256];
void rc4(const u8 *in, int len, const u8 *key, int key_len, u8 *out)
{
    u8 *d = s;
    u8 tmp, tmp2;
    int i1, i2, j;
    i2 = j = 0;
#define LOOP1(n)                        \
    {                                   \
        tmp = d[n];                     \
        j = (j + tmp + key[i2]) & 0xff; \
        if (++i2 == key_len)            \
            i2 = 0;                     \
        d[n] = d[j];                    \
        d[j] = tmp;                     \
    }

    for (i1 = 0; i1 < 256; i1++)
        d[i1] = i1;
    for (i1 = 0; i1 < 256; i1 += 4)
    {
        LOOP1(i1);
        LOOP1(i1 + 1);
        LOOP1(i1 + 2);
        LOOP1(i1 + 3);
    }
    i1 = i2 = 0;

#define LOOP2(in, out)                     \
    {                                      \
        i1 = ((i1 + 1) & 0xff);            \
        tmp = d[i1];                       \
        i2 = (tmp + i2) & 0xff;            \
        d[i1] = tmp2 = d[i2];              \
        d[i2] = tmp;                       \
        out = d[(tmp + tmp2) & 0xff] ^ in; \
    }

    int len_ = len >> 4;
    while (len_)
    {
        LOOP2(in[0], out[0]);
        LOOP2(in[1], out[1]);
        LOOP2(in[2], out[2]);
        LOOP2(in[3], out[3]);
        LOOP2(in[4], out[4]);
        LOOP2(in[5], out[5]);
        LOOP2(in[6], out[6]);
        LOOP2(in[7], out[7]);
        LOOP2(in[8], out[8]);
        LOOP2(in[9], out[9]);
        LOOP2(in[10], out[10]);
        LOOP2(in[11], out[11]);
        LOOP2(in[12], out[12]);
        LOOP2(in[13], out[13]);
        LOOP2(in[14], out[14]);
        LOOP2(in[15], out[15]);
        in += 16;
        out += 16;
        len_--;
    }
    len_ = len & 0x0f;
    for (int i = 0; i < len_; i++)
    {
        LOOP2(in[i], out[i]);
    }
}

void test()
{
    int num_bytes = 16 * 1024 / 8;
    u8 *text = new u8[num_bytes];
    u8 *key = new u8[128];
    u8 *buffer = new u8[num_bytes];
    u8 *target = new u8[num_bytes];
    // test 10 random 16Kbits input
    RC4_KEY rc4_key;
    for (int i = 0; i < 10; i++)
    {
        random_fill(text, num_bytes);
        random_fill(key, 128);
        rc4(text, num_bytes, key, 128, buffer);
        RC4_set_key(&rc4_key, 128, key);
        RC4(&rc4_key, num_bytes, text, target);
        assert_eq(buffer, target, num_bytes);
    }
}

void bench()
{
    int num_bytes = 16 * 1024 / 8;
    int times = 1024;
    u8 *text = new u8[num_bytes];
    u8 *key = new u8[128];
    u8 *buffer = new u8[num_bytes];
    random_fill(text, num_bytes);
    random_fill(key, 128);
    double t0 = clock();
    for (int i = 0; i < times; i++)
    {
        rc4(text, num_bytes, key, 128, buffer);
    }
    double tm = (clock() - t0) / CLOCKS_PER_SEC;
    printf("%lf Mbps\n", num_bytes * 8 * times / 1024 / 1024 / tm);
}

void example()
{
    int num_bytes = 16 * 1024 / 8;
    u8 *text = new u8[num_bytes];
    u8 *key = new u8[128];
    u8 *buffer = new u8[num_bytes];
    random_fill(text, num_bytes);
    random_fill(key, 128);
    rc4(text, num_bytes, key, 128, buffer);
    printf("input: \n%s\n", to_hex(text, num_bytes).c_str());
    printf("key: \n%s\n", to_hex(key, 128).c_str());
    printf("output: \n%s\n", to_hex(buffer, num_bytes).c_str());
}

int main()
{
    test();
    bench();
    example();
    return 0;
}