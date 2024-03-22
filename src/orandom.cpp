#include "orandom.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sys/random.h>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cmath>

using namespace std;

using namespace Random;

inline std::string toLower(const std::string &str)
{
    std::string newstr;
    std::ranges::for_each(str, [&newstr](char c)
                          { newstr.push_back(std::tolower(c)); });
    return newstr;
}

inline char select_char(char from, char to, unsigned char salt)
{
    const unsigned count_chars = to - from + 1;
    return from + (salt % count_chars);
}

bool GetRandom::refill_buffer()
{
    const ssize_t read = getrandom(rd_buf, buf_size, 0);
    current_index = 0;
    if (read < 0)
    { // an error occurred
        bytes_read = 0;
        return false;
    }
    else
    {
        bytes_read = read;
    }
    return true;
}

unsigned char GetRandom::get()
{
    while (current_index >= bytes_read)
    {
        if (!refill_buffer())
        {
            return '0'; // return default char if buffer could not be refilled
        }
    }
    return rd_buf[current_index++];
}

void GetRandom::shuffle(std::string &string_ref, unsigned shuffle_times)
{
    const unsigned strsize = string_ref.size();
    while (shuffle_times-- > 0)
    {
        for (unsigned i = 0; i < strsize; ++i)
        {
            const unsigned char salt = get();
            const unsigned j = salt % strsize;
            const char tmp = string_ref.at(i);
            string_ref.at(i) = string_ref.at(j);
            string_ref.at(j) = tmp;
        }
    }
}

unsigned GetRandom::generate_int(int min_value, int max_value)
{
    min_value = std::abs(min_value);
    max_value = std::abs(max_value);
    if (min_value == max_value)
    {
        return max_value;
    }
    else if (max_value < min_value)
    {
        int tmp = min_value;
        min_value = max_value;
        max_value = tmp;
    }
    const unsigned pool_size = max_value - min_value + 1;
    const unsigned threshold = pool_size * 5;
    unsigned salt = get();
    while (salt < threshold)
    {
        salt = salt * get() + 1; // enable prime numbers
    }
    const unsigned add = salt % pool_size;
    return min_value + add;
}

const std::string CharPool::excluded_specials{"\"`'.,^"};

bool CharPool::contains(const std::string &str, char c)
{
    return str.find(c) != std::string::npos;
}

char CharPool::get_from_pool(const CharVec &vec)
{
    if (m_is_dirty)
        populatePools();
    if (vec.empty())
        return '?'; // should not happen
    unsigned index = m_get_random.generate_int(0, vec.size() - 1);
    return vec.at(index);
}

void CharPool::exclude_chars(const string &excl)
{
    m_excluded_chars = excl;
    m_is_dirty = true;
}

void CharPool::populatePools()
{
    static auto mypopulate = [this](CharVec &vec, char from, char to)
    {
        vec.clear();
        for (char c = from; c <= to; ++c)
        {
            if (!contains(m_excluded_chars, c))
            {
                vec.push_back(c);
                m_all.push_back(c);
            }
        }
    };
    m_all.clear();
    mypopulate(m_lowercase, 'a', 'z');
    mypopulate(m_uppercase, 'A', 'Z');
    mypopulate(m_digits, '0', '9');
    m_specials.clear();
    for (char c = '!'; c <= '~'; ++c)
    {
        if (std::ispunct(c) && !contains(m_excluded_chars, c) && !contains(excluded_specials, c))
        {
            m_specials.push_back(c);
            m_all.push_back(c);
        }
    }
    m_is_dirty = false;
}

std::string PasswordGenerator::get_password()
{
    std::string pw;
    unsigned my_lc = m_min_lc;
    unsigned my_uc = m_min_uc;
    unsigned my_dig = m_min_dig;
    unsigned my_spec = m_min_spec;
    static auto trim_number = [this](unsigned &number) { // test if static makes sense here!
        unsigned max = m_length / 4;
        if (number > max)
            number = max;
    };
    // check if sum of minimum numbers exceeds password length:
    if ((my_lc + my_uc + my_dig + my_spec) > m_length)
    {
        trim_number(my_uc);
        trim_number(my_dig);
        trim_number(my_spec);
        my_lc = m_length - my_uc - my_dig - my_spec;
    }
    // start creating password:
    for (unsigned i = 0; i < my_lc; ++i)
    {
        pw.push_back(m_charpool.get_from_lc());
    }
    for (unsigned i = 0; i < my_uc; ++i)
    {
        pw.push_back(m_charpool.get_from_uc());
    }
    for (unsigned i = 0; i < my_dig; ++i)
    {
        pw.push_back(m_charpool.get_from_digits());
    }
    for (unsigned i = 0; i < my_spec; ++i)
    {
        pw.push_back(m_charpool.get_from_specials());
    }
    while (pw.size() < m_length)
    {
        pw.push_back(m_charpool.get_from_all());
    }
    m_get_random.shuffle(pw, 20);
    return pw;
}
