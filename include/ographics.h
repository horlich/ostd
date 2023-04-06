#ifndef OGRAPHICS_H_INCLUDED
#define OGRAPHICS_H_INCLUDED


#include <array>
#include <string>
#include <iostream>



namespace OGraphics {
using namespace std;


class OColor {

public:
    explicit OColor(unsigned long value = 0);
    OColor(unsigned char red, unsigned char green, unsigned char blue);
    explicit OColor(const std::string& format);

    inline unsigned long value() const { return m_value; }
    inline int red() const   { return m_rgb_array[2]; }
    inline int green() const { return m_rgb_array[1]; }
    inline int blue() const  { return m_rgb_array[0]; }
    OColor complementary() const;

private:
    unsigned long m_value = 0;
    unsigned char* m_rgb_array = reinterpret_cast<unsigned char*>(&m_value);  // array index: red=2, green=1, blue=0
};

ostream& operator<<(ostream & os, const OColor& color);

void print_color_info(const OColor&, std::ostream& = std::cout);

} // namespace OGraphics


#endif // OGRAPHICS_H_INCLUDED
