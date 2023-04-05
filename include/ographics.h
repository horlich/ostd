#ifndef OGRAPHICS_H_INCLUDED
#define OGRAPHICS_H_INCLUDED


#include <array>
#include <string>



namespace OGraphics {
using namespace std;


class OColor {
    struct ColorComponent {
        //
        ColorComponent(unsigned char _value = 0) : value{_value} {}
        unsigned char complementary() const { return 0xFF - value; }
        unsigned char value {0};
    };

public:
    OColor(unsigned long value = 0);
    OColor(unsigned char red, unsigned char green, unsigned char blue);
    OColor(const std::string& format);

    unsigned long value() const { return m_value; }

    unsigned char red() const   { return m_rgb_array[0].value; }
    unsigned char green() const { return m_rgb_array[1].value; }
    unsigned char blue() const  { return m_rgb_array[2].value; }

    OColor complementary() const;

private:
    void create_value();

    unsigned long m_value = 0;
    std::array<ColorComponent, 3> m_rgb_array;
};

ostream& operator<<(ostream & os, const OColor& color);

} // namespace OGraphics


#endif // OGRAPHICS_H_INCLUDED
