#include <sys/random.h>
#include <string>
#include <vector>

namespace Random
{

    class GetRandom
    {
    public:
        static constexpr unsigned buf_size{100};

        unsigned char get();
        void shuffle(std::string &string_ref, unsigned shuffle_times);
        unsigned generate_int(int min_value, int max_value);

    private:
        unsigned char rd_buf[buf_size];
        bool refill_buffer();

        unsigned bytes_read{0};
        unsigned current_index{0};
    };

    class CharPool
    {
    public:
        using CharVec = std::vector<char>;

        static const std::string excluded_specials;

        void exclude_chars(const std::string &);
        inline char get_from_lc() { return get_from_pool(m_lowercase); }
        inline char get_from_uc() { return get_from_pool(m_uppercase); }
        inline char get_from_digits() { return get_from_pool(m_digits); }
        inline char get_from_specials() { return get_from_pool(m_specials); }
        inline char get_from_all() { return get_from_pool(m_all); }

    private:
        void populatePools();
        bool contains(const std::string &, char);
        char get_from_pool(const CharVec &);

        CharVec m_lowercase;
        CharVec m_uppercase;
        CharVec m_digits;
        CharVec m_specials;
        CharVec m_all;
        bool m_is_dirty{true};
        std::string m_excluded_chars;
        GetRandom m_get_random;
    };

    class PasswordGenerator
    {
    public:
        inline void set_excluded_chars(const std::string &excl) { m_charpool.exclude_chars(excl); }
        inline void set_password_length(unsigned len) { m_length = len; }
        inline void set_min_lc(unsigned val) { m_min_lc = val; }
        inline void set_min_uc(unsigned val) { m_min_uc = val; }
        inline void set_min_dig(unsigned val) { m_min_dig = val; }
        inline void set_min_spec(unsigned val) { m_min_spec = val; }

        std::string get_password();

    private:
        unsigned m_length{12};
        unsigned m_min_lc{1};
        unsigned m_min_uc{1};
        unsigned m_min_dig{1};
        unsigned m_min_spec{1};
        GetRandom m_get_random;
        CharPool m_charpool;
    };

} // namespace Random
