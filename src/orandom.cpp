// #include "../include/orandom.h"
#include "orandom.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sys/random.h>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cmath>

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

void PasswordGenerator::set_valid_specials(const std::string &&str)
{
    m_valid_specials.clear();
    for (char c : str)
    {
        if (std::ispunct(c))
            m_valid_specials.push_back(c);
    }
}

std::string PasswordGenerator::get_password()
{
    std::string pw;
    const bool all_specials_valid = ((toLower(m_valid_specials) == "all") || (m_min_spec && m_valid_specials.empty()));
    unsigned lc_left = m_min_lc;
    unsigned uc_left = m_min_uc;
    unsigned dig_left = m_min_dig;
    unsigned spec_left = m_min_spec;
    static auto trim_number = [this](unsigned &number) { // test if static makes sense here!
        unsigned max = m_length / 4;
        if (number > max)
            number = max;
    };
    // check if sum of minimum numbers exceeds password length:
    if ((lc_left + uc_left + dig_left + spec_left) > m_length)
    {
        trim_number(uc_left);
        trim_number(dig_left);
        trim_number(spec_left);
        lc_left = m_length - uc_left - dig_left - spec_left;
    }
    // start creating password:
    char selected_char;
    while (pw.length() < m_length)
    {
        const unsigned char salt = m_get_random.get();
        if (lc_left > 0)
        {
            selected_char = select_char('a', 'z', salt);
            --lc_left;
        }
        else if (uc_left > 0)
        {
            selected_char = select_char('A', 'Z', salt);
            --uc_left;
        }
        else if (dig_left > 0)
        {
            selected_char = select_char('0', '9', salt);
            --dig_left;
        }
        else if (spec_left > 0)
        {
            if (all_specials_valid)
            {
                if (std::ispunct(salt))
                {
                    selected_char = salt;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                selected_char = m_valid_specials.at(salt %  m_valid_specials.size());
            }
            --spec_left;
        }
        else if (std::isalnum(salt))
        {
            selected_char = salt;
        }
        else if (std::ispunct(salt))
        {
            if (all_specials_valid)
            {
                selected_char = salt;
            }
            else
            {
                size_t pos = m_valid_specials.find(salt);
                if (pos != std::string::npos)
                {
                    selected_char = m_valid_specials.at(pos);
                }
                else
                {
                    continue; // unsupported special character
                }
            }
        }
        else
        {
            continue; // needless byte
        }
        pw.push_back(selected_char);
    }
    m_get_random.shuffle(pw, 20);
    return pw;
}
