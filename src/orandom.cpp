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

inline std::string toLower(const std::string &str)
{
    std::string newstr;
    std::ranges::for_each(str, [&newstr](char c)
                          { newstr.push_back(std::tolower(c)); });
    return newstr;
}

inline char select_char(char from, char to, unsigned char salt)
{
    unsigned count_chars = to - from + 1;
    return from + (salt % count_chars);
}

bool GetRandom::refill_buffer()
{
    ssize_t read = getrandom(rd_buf, buf_size, 0);
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
    unsigned strsize = string_ref.size();
    while (shuffle_times-- > 0)
    {
        for (unsigned i = 0; i < strsize; ++i)
        {
            unsigned char salt = get();
            unsigned j = salt % strsize;
            char tmp = string_ref.at(i);
            string_ref.at(i) = string_ref.at(j);
            string_ref.at(j) = tmp;
        }
    }
}

unsigned GetRandom::generate_int(int min_value, int max_value)
{
    min_value = std::abs(min_value);
    max_value = std::abs(max_value);
    if (min_value == max_value) {
        return max_value;
    } else if (max_value < min_value) {
        int tmp = min_value;
        min_value = max_value;
        max_value = tmp;
    }
    unsigned pool_size = max_value - min_value + 1;
    unsigned salt = get();
    unsigned add = (salt + max_value) % pool_size;
    return min_value + add;
}


std::string PasswordGenerator::get_password()
{
    std::string pw;
    bool all_specials_valid = ((toLower(valid_specials) == "all") || (min_spec > valid_specials.size()));
    unsigned lc_left = min_lc;
    unsigned uc_left = min_uc;
    unsigned dig_left = min_dig;
    unsigned spec_left = min_spec;
    char selected_char;
    while (pw.length() < length)
    {
        unsigned char salt = get_random.get();
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
                selected_char = valid_specials.at(salt % valid_specials.size());
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
                size_t pos = valid_specials.find(salt);
                if (pos != std::string::npos)
                {
                    selected_char = valid_specials.at(pos);
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
    get_random.shuffle(pw, 20);
    return pw;
}

