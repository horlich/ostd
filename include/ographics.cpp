#include "ographics.h"
#include <stdexcept>

using namespace std;
using namespace OGraphics;

OColor::OColor(unsigned long rgb) : m_value(rgb)
{
    if (rgb > 0xFFFFFF)
        throw range_error("OColor: Value must be in the range from 0 to 0xFFFFFF");
    m_rgb_array = reinterpret_cast<unsigned char*>(&m_value);
}

OColor::OColor(unsigned char red, unsigned char green, unsigned char blue)
{
    m_rgb_array = reinterpret_cast<unsigned char*>(&m_value);
    m_rgb_array[2] = red;
    m_rgb_array[1] = green;
    m_rgb_array[0] = blue;
}

bool check_rgb_code__(const string& code)
{
    if (code.length() != 7)
        return false;
    if (code[0] != '#')
        return false;
    for (int i = 1; i < code.length(); ++i) {
        if (! std::isxdigit(code[i]))
            return false;
    }
    return true;
}

OColor::OColor(const std::string& rgb_code)
{
    if (! check_rgb_code__(rgb_code))
        throw  invalid_argument("malformated color string");
    m_value = stoul(rgb_code.substr(1), 0, 16);
    m_rgb_array = reinterpret_cast<unsigned char*>(&m_value);
}

OColor OColor::complementary() const
{
    return OColor(0xFF - red(), 0xFF - green(), 0xFF - blue());
}

ostream& OGraphics::operator<<(ostream & os, const OColor& color)
{
    os << "OColor(Hex 0x" << hex << color.value() << "; Dec " << dec << int(color.red()) << "," << int(color.green()) << "," << int(color.blue()) << ")";
    return os;
}

void OGraphics::print_color_info(const OColor& color, std::ostream& os)
{
    os << "        Color: " << color << endl;
    OGraphics::OColor complementary = color.complementary();
    os << "Complementary: " << complementary << endl;
}
