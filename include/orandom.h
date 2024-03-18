#include <sys/random.h>
#include <string>



class GetRandom
{
public:
    static constexpr unsigned buf_size{100};

    unsigned char get();
    void shuffle(std::string& string_ref, unsigned shuffle_times);
    unsigned generate_int(int min_value, int max_value);

private:
    unsigned char rd_buf[buf_size];
    bool refill_buffer();

    unsigned bytes_read{0};
    unsigned current_index{0};
};



class PasswordGenerator
{
public:
    std::string get_password();

private:
    unsigned length     {12};
    unsigned min_lc     {1};
    unsigned min_uc     {1};
    unsigned min_dig    {1};
    unsigned min_spec   {1};
    std::string valid_specials;
    GetRandom get_random;
};
