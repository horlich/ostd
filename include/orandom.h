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
    void set_valid_specials(const std::string&&);
    inline void set_password_length(unsigned len) { m_length = len; } 
    inline void set_min_lc(unsigned val) { m_min_lc = val; }
    inline void set_min_uc(unsigned val) { m_min_uc = val; }
    inline void set_min_dig(unsigned val) { m_min_dig = val; }
    inline void set_min_spec(unsigned val) { m_min_spec = val; }

    std::string get_password();

private:
    unsigned m_length   {12};
    unsigned m_min_lc     {1};
    unsigned m_min_uc     {1};
    unsigned m_min_dig    {1};
    unsigned m_min_spec   {1};
    std::string m_valid_specials;
    GetRandom m_get_random;
};
