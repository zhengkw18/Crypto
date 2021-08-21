#include <vector>
#include <string>
#include <assert.h>
#include <stdio.h>
#include <gmp.h>
#include "time.h"
#include <set>

// sudo apt install libgmp3-dev
// g++ rm.cpp -o rm -O2 -lgmp -lgmpxx

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

struct BigUInt
{
    std::vector<u32> digits;
    BigUInt() {}
    // base=2
    BigUInt(u32 d)
    {
        digits.push_back(d);
        trim();
    }
    inline void trim()
    {
        while (digits.size() && digits.back() == 0)
            digits.pop_back();
    }
    int length()
    {
        return digits.empty() ? 0 : (32 * digits.size() - __builtin_clz(digits.back()));
    }
    std::string to_binary() const
    {
        if (digits.empty())
            return "0";
        std::string s;
        for (int i = 0; i < digits.size() - 1; i++)
        {
            u32 d = digits[i];
            for (int j = 0; j < 32; j++)
            {
                s = (d & 1 ? "1" : "0") + s;
                d >>= 1;
            }
        }
        u32 d = digits.back();
        while (d)
        {
            s = (d & 1 ? "1" : "0") + s;
            d >>= 1;
        }
        return s;
    }
    std::string to_hex() const
    {
        const static std::string chars("0123456789abcdef");
        if (digits.empty())
            return "0";
        std::string s;
        for (int i = 0; i < digits.size() - 1; i++)
        {
            u32 d = digits[i];
            for (int j = 0; j < 8; j++)
            {
                s = chars[d & 0xf] + s;
                d >>= 4;
            }
        }
        u32 d = digits.back();
        while (d)
        {
            s = chars[d & 0xf] + s;
            d >>= 4;
        }
        return s;
    }
    void print() const
    {
        for (u32 digit : digits)
        {
            printf("%u ", digit);
        }
        printf("\n");
    }
    BigUInt(std::string s)
    {
        int len = s.size();
        while (len > 0)
        {
            int start = std::max(0, len - 32);
            u32 digit = 0;
            for (int i = start; i < len; i++)
            {
                digit <<= 1;
                digit |= (s[i] == '1') ? 1 : 0;
            }
            digits.push_back(digit);
            len -= 32;
        }
        trim();
    }
    inline bool operator==(const BigUInt &b) const
    {
        if (digits.size() != b.digits.size())
            return false;
        for (int i = 0; i < digits.size(); i++)
            if (digits[i] != b.digits[i])
                return false;
        return true;
    }
    inline bool operator!=(const BigUInt &b) const
    {
        return !(*this == b);
    }
    inline bool operator<(const BigUInt &b) const
    {
        if (digits.size() != b.digits.size())
            return digits.size() < b.digits.size();
        for (int i = digits.size() - 1; i >= 0; i--)
            if (digits[i] != b.digits[i])
                return digits[i] < b.digits[i];
        return false;
    }
    inline bool operator<=(const BigUInt &b) const
    {
        if (digits.size() != b.digits.size())
            return digits.size() < b.digits.size();
        for (int i = digits.size() - 1; i >= 0; i--)
            if (digits[i] != b.digits[i])
                return digits[i] < b.digits[i];
        return true;
    }
    inline bool operator>(const BigUInt &b) const
    {
        return !(*this <= b);
    }
    inline bool operator>=(const BigUInt &b) const
    {
        return !(*this < b);
    }
    BigUInt operator+(const BigUInt &b) const
    {
        BigUInt c;
        u32 carry = 0;
        for (int i = 0; i < digits.size() || i < b.digits.size(); i++)
        {
            u64 re = carry + (u64)(i < digits.size() ? digits[i] : 0) + (u64)(i < b.digits.size() ? b.digits[i] : 0);
            c.digits.push_back(re & 0xffffffff);
            carry = re >> 32;
        }
        c.digits.push_back(carry);
        c.trim();
        return c;
    }
    BigUInt operator-(const BigUInt &b) const
    {
        assert(*this > b);
        BigUInt c;
        bool carry = 0;
        for (int i = 0; i < digits.size(); i++)
        {
            i64 re = (i64)digits[i] - (i64)(i < b.digits.size() ? b.digits[i] : 0) - carry;
            c.digits.push_back(re & 0xffffffff);
            carry = (re < 0);
        }
        c.trim();
        return c;
    }
    BigUInt operator*(const BigUInt &b) const
    {
        BigUInt c;
        c.digits.resize(digits.size() + b.digits.size());
        for (int i = 0; i < digits.size(); i++)
        {
            u64 k = 0;
            for (int j = 0; j < b.digits.size(); j++)
            {
                k = k + c.digits[i + j] + (u64)digits[i] * b.digits[j];
                c.digits[i + j] = k & 0xffffffff;
                k >>= 32;
            }
            c.digits[i + b.digits.size()] = k;
        }
        c.trim();
        return c;
    }
    BigUInt operator/(const BigUInt &b) const
    {
        BigUInt div, mod;
        divi(*this, b, div, mod);
        return div;
    }
    BigUInt operator%(const BigUInt &b) const
    {
        BigUInt div, mod;
        divi(*this, b, div, mod);
        return mod;
    }
#define __trial_division_subt()                                                               \
    {                                                                                         \
        bool positive = 1;                                                                    \
        u64 k = 0;                                                                            \
        for (int j = 0; j < b.digits.size(); j++)                                             \
        {                                                                                     \
            if (positive)                                                                     \
            {                                                                                 \
                if (k + mod.digits[i + j] >= 1ull * p * b.digits[j])                          \
                {                                                                             \
                    k = k + mod.digits[i + j] - 1ull * p * b.digits[j];                       \
                    positive = 1;                                                             \
                }                                                                             \
                else                                                                          \
                {                                                                             \
                    k = 1ull * p * b.digits[j] - k - mod.digits[i + j];                       \
                    positive = 0;                                                             \
                }                                                                             \
            }                                                                                 \
            else                                                                              \
            {                                                                                 \
                if (mod.digits[i + j] >= 1ull * p * b.digits[j] + k)                          \
                {                                                                             \
                    k = mod.digits[i + j] - 1ull * p * b.digits[j] - k;                       \
                    positive = 1;                                                             \
                }                                                                             \
                else                                                                          \
                {                                                                             \
                    k = 1ull * p * b.digits[j] + k - mod.digits[i + j];                       \
                    positive = 0;                                                             \
                }                                                                             \
            }                                                                                 \
            mod.digits[i + j] = positive ? k & 0xffffffff : -(k & 0xffffffff);                \
            if (!positive)                                                                    \
                k += 0xffffffff;                                                              \
            k >>= 32;                                                                         \
        }                                                                                     \
        if (k)                                                                                \
            mod.digits[i + b.digits.size()] += positive ? k & 0xffffffff : -(k & 0xffffffff); \
        div.digits[i] += p;                                                                   \
    }

#define __get_val(x, y) ((((u64)((y) + 1 < (x.digits.size()) ? x.digits[(y) + 1] : 0)) << 32) + (u64)x.digits[(y)])
    friend void divi(const BigUInt &a, const BigUInt &b, BigUInt &div, BigUInt &mod)
    {
        div = 0;
        mod = a;
        if (a < b)
        {
            return;
        }
        div.digits.resize(a.digits.size() - b.digits.size() + 1);
        for (int i = a.digits.size() - b.digits.size(); i >= 0; i--)
        {
            u32 p;
            while (p = __get_val(mod, i + b.digits.size() - 1) / (__get_val(b, b.digits.size() - 1) + 1))
            {
                __trial_division_subt();
            }
            p = 1;
            for (int j = b.digits.size() - 1; j >= 0; j--)
                if (mod.digits[j + i] != b.digits[j])
                {
                    p = b.digits[j] < mod.digits[j + i];
                    break;
                }
            if (p)
                __trial_division_subt();
        }
        div.trim();
        mod.trim();
    }
};

BigUInt gcd(BigUInt a, BigUInt b)
{
    BigUInt t;
    while (b != 0)
    {
        t = b;
        b = a % b;
        a = t;
    }
    return a;
}

BigUInt modexp(BigUInt a, BigUInt b, BigUInt n)
{
    BigUInt c = 1;
    while (b > 1)
    {
        if (b % 2 == 0)
        {
            a = a * a;
            a = a % n;
            b = b / 2;
        }
        else
        {
            c = c * a;
            c = c % n;
            b = b - 1;
        }
    }
    return (a * c) % n;
}

std::string random_odd(int length)
{
    assert(length >= 2);
    std::string s = "1";
    for (int i = 0; i < length - 2; i++)
    {
        if (rand() & 1)
            s.append("1");
        else
            s.append("0");
    }
    s.append("1");
    return s;
}
std::string random_num(int length)
{
    std::string s;
    for (int i = 0; i < length; i++)
    {
        if (rand() & 1)
            s.append("1");
        else
            s.append("0");
    }
    return s;
}

void test()
{
    mpz_t s1, s2, r;
    for (int i = 0; i < 1000; i++)
    {
        std::string a1 = random_num(10000), a2 = random_num(5000);
        mpz_init_set_str(s1, a1.c_str(), 2);
        mpz_init_set_str(s2, a2.c_str(), 2);
        mpz_init(r);
        BigUInt ss1(a1.c_str()), ss2(a2.c_str()), rr;
        // test +
        mpz_add(r, s1, s2);
        rr = ss1 + ss2;
        assert(rr.to_hex().c_str() == std::string(mpz_get_str(nullptr, 16, r)));
        // test -
        mpz_sub(r, s1, s2);
        rr = ss1 - ss2;
        assert(rr.to_hex().c_str() == std::string(mpz_get_str(nullptr, 16, r)));
        // test *
        mpz_mul(r, s1, s2);
        rr = ss1 * ss2;
        assert(rr.to_hex().c_str() == std::string(mpz_get_str(nullptr, 16, r)));
        if (ss2 == 0)
            continue;
        // test /
        mpz_div(r, s1, s2);
        rr = ss1 / ss2;
        assert(rr.to_hex().c_str() == std::string(mpz_get_str(nullptr, 16, r)));
        // test %
        assert(rr * ss2 + ss1 % ss2 == ss1);
        mpz_clear(r);
        mpz_clear(s1);
        mpz_clear(s2);
    }
}

void bench_add()
{
    std::string a1 = random_num(10000), a2 = random_num(5000);
    mpz_t s1, s2, r;
    mpz_init_set_str(s1, a1.c_str(), 2);
    mpz_init_set_str(s2, a2.c_str(), 2);
    mpz_init(r);
    BigUInt ss1(a1.c_str()), ss2(a2.c_str()), rr;
    double t0 = clock();
    for (int i = 0; i < 1000; i++)
    {
        rr = ss1 + ss2;
    }
    double tm1 = clock() - t0;
    t0 = clock();
    for (int i = 0; i < 1000; i++)
    {
        mpz_add(r, s1, s2);
    }
    double tm2 = clock() - t0;
    mpz_clear(r);
    mpz_clear(s1);
    mpz_clear(s2);
    printf("gmp add is %lfx times faster\n", tm1 / tm2);
}

void bench_sub()
{
    std::string a1 = random_num(10000), a2 = random_num(5000);
    mpz_t s1, s2, r;
    mpz_init_set_str(s1, a1.c_str(), 2);
    mpz_init_set_str(s2, a2.c_str(), 2);
    mpz_init(r);
    BigUInt ss1(a1.c_str()), ss2(a2.c_str()), rr;
    double t0 = clock();
    for (int i = 0; i < 1000; i++)
    {
        rr = ss1 - ss2;
    }
    double tm1 = clock() - t0;
    t0 = clock();
    for (int i = 0; i < 1000; i++)
    {
        mpz_sub(r, s1, s2);
    }
    double tm2 = clock() - t0;
    mpz_clear(r);
    mpz_clear(s1);
    mpz_clear(s2);
    printf("gmp sub is %lfx times faster\n", tm1 / tm2);
}

void bench_mul()
{
    std::string a1 = random_num(10000), a2 = random_num(5000);
    mpz_t s1, s2, r;
    mpz_init_set_str(s1, a1.c_str(), 2);
    mpz_init_set_str(s2, a2.c_str(), 2);
    mpz_init(r);
    BigUInt ss1(a1.c_str()), ss2(a2.c_str()), rr;
    double t0 = clock();
    for (int i = 0; i < 1000; i++)
    {
        rr = ss1 * ss2;
    }
    double tm1 = clock() - t0;
    t0 = clock();
    for (int i = 0; i < 1000; i++)
    {
        mpz_mul(r, s1, s2);
    }
    double tm2 = clock() - t0;
    mpz_clear(r);
    mpz_clear(s1);
    mpz_clear(s2);
    printf("gmp mul is %lfx times faster\n", tm1 / tm2);
}

void bench_div()
{
    std::string a1 = random_num(10000), a2 = random_num(5000);
    mpz_t s1, s2, r;
    mpz_init_set_str(s1, a1.c_str(), 2);
    mpz_init_set_str(s2, a2.c_str(), 2);
    mpz_init(r);
    BigUInt ss1(a1.c_str()), ss2(a2.c_str()), rr;
    double t0 = clock();
    for (int i = 0; i < 1000; i++)
    {
        rr = ss1 / ss2;
    }
    double tm1 = clock() - t0;
    t0 = clock();
    for (int i = 0; i < 1000; i++)
    {
        mpz_div(r, s1, s2);
    }
    double tm2 = clock() - t0;
    mpz_clear(r);
    mpz_clear(s1);
    mpz_clear(s2);
    printf("gmp div is %lfx times faster\n", tm1 / tm2);
}

void bench()
{
    bench_add();
    bench_sub();
    bench_mul();
    bench_div();
}

bool test_prime(BigUInt n, int t)
{
    if (n == 2)
        return true;
    if (n == 1 || n % 2 == 0)
        return false;
    BigUInt d = n - 1;
    std::set<BigUInt> bases;
    while (bases.size() < t)
    {
        BigUInt a = random_num(n.length());
        if (a > 1 && a < d)
            bases.insert(a);
    }
    u32 s = 0;
    BigUInt r = d, x;
    // n - 1 = r * 2^s
    while (r % 2 == 0)
    {
        s = s + 1;
        r = r / 2;
    }
    for (const BigUInt &a : bases)
    {
        if (gcd(a, n) != 1)
        {
            return false;
        }
        x = modexp(a, r, n);
        if ((x == 1) || (x == d))
            continue;
        for (int k = 1; k < s; k++)
        {
            x = (x * x) % n;
            if (x == d)
                break;
            if (x == 1)
                return false;
        }
        if (x != d)
            return false;
    }
    return true;
}

int main()
{
    test();
    bench();
    int rounds = 20;
    BigUInt n;
    n = random_odd(512);
    bool is_prime = test_prime(n, rounds);
    printf("Testing %s\n", n.to_binary().c_str());
    if (is_prime)
        printf("Result: prime\n");
    else
        printf("Result: not prime\n");
    if (!is_prime)
    {
        printf("Searching for prime...\n");
        int cnt = 0;
        while (1)
        {
            cnt++;
            n = random_odd(512);
            if (test_prime(n, rounds))
            {
                printf("%d numbers searched\n", cnt);
                printf("Find a prime %s\n", n.to_binary().c_str());
                break;
            }
        }
    }
    return 0;
}