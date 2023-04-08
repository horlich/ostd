#ifndef OGRAPHICS_H_INCLUDED
#define OGRAPHICS_H_INCLUDED


#include <array>
#include <string>
#include <iostream>


using namespace std;

namespace OGraphics {


/*
    OColor: See article 'Color Models: RGB, HSV, HSL'
            in https://en.wikibooks.org/wiki/Color_Models:_RGB,_HSV,_HSL
 */

class OColor {

public:
    explicit OColor(unsigned long value = 0);
    OColor(unsigned char red, unsigned char green, unsigned char blue);
    explicit OColor(const std::string& format);

    unsigned long value() const;
    inline int red() const   { return m_red; }
    inline int green() const { return m_green; }
    inline int blue() const  { return m_blue; }
    std::string to_string() const;
    OColor complementary() const;
    OColor darker(float shade_factor) const;
    OColor brighter(float tint_factor) const;

private:
    unsigned char m_red   = 0;
    unsigned char m_green = 0;
    unsigned char m_blue  = 0;
};

ostream& operator<<(ostream & os, const OColor& color);

void print_color_info(const OColor&, std::ostream& = std::cout);

} // namespace OGraphics


#endif // OGRAPHICS_H_INCLUDED
