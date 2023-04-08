#include "ographics.h"
#include <stdexcept>
#include <cmath>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace OGraphics;

OColor::OColor(unsigned long rgb)
{
    if (rgb > 0xFFFFFF)
        throw range_error("OColor: Value must be in the range from 0 to 0xFFFFFF");
    unsigned char* arr = reinterpret_cast<unsigned char*>(&rgb);
    m_red   = arr[2];
    m_green = arr[1];
    m_blue  = arr[0];
}

OColor::OColor(unsigned char red, unsigned char green, unsigned char blue)
    : m_red(red), m_green(green), m_blue(blue) {}

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
    m_red   = stoul(rgb_code.substr(1,2), 0, 16);
    m_green = stoul(rgb_code.substr(3,2), 0, 16);
    m_blue  = stoul(rgb_code.substr(5,2), 0, 16);
}

unsigned long OColor::value() const
{
    unsigned long val {0};
    unsigned char* arr = reinterpret_cast<unsigned char*>(&val);
    arr[2] = m_red;
    arr[1] = m_green;
    arr[0] = m_blue;
    return val;
}

string OColor::to_string() const {
    stringstream buf;
    buf.put('#');
    buf << hex << setw(2) << setfill('0') << int(m_red);
    buf << hex << setw(2) << setfill('0') << int(m_green);
    buf << hex << setw(2) << setfill('0') << int(m_blue);
    return buf.str();
}

OColor OColor::complementary() const
{
    return OColor(0xFF - m_red, 0xFF - m_green, 0xFF - m_blue);
}

unsigned char component_darker__(float intensity, float shade_factor)
{
    return std::lround(intensity * (1.0 - shade_factor));
}


/*
    Helper classes for shading and tinting
 */

class Change_factor {
public:
    Change_factor(float _factor = 0.0)
    {
        if (_factor > 1.0)
            _factor = 1.0;
        if (_factor < 0.0)
            _factor = 0.0;
        factor = _factor;
    }
protected:
    float factor = 0.0;
};

class Darkener : public Change_factor {
/*
    new intensity = current intensity * (1 - shade factor)
 */
public:
    Darkener(float shade_factor) : Change_factor(shade_factor) {}
    unsigned char operator()(float current_intensity)
    {
       return std::lround(current_intensity * (1.0 - factor));
    }
};

class Brightener : public Change_factor {
/*
    new intensity = current intensity + (255 - current intensity) * tint factor
 */
public:
    Brightener(float tint_factor) : Change_factor(tint_factor) {}
    unsigned char operator()(float current_intensity)
    {
       return std::lround(current_intensity + (float(0xFF) - current_intensity) * factor);
    }
};

OColor OColor::darker(float shade_factor) const
{
    Darkener darken(shade_factor);
    return OColor(darken(m_red), darken(m_green), darken(m_blue));
}

OColor OColor::brighter(float tint_factor) const
{
    Brightener brighten(tint_factor);
    return OColor(brighten(m_red), brighten(m_green), brighten(m_blue));
}




ostream& OGraphics::operator<<(ostream & os, const OColor& color)
{
    os << "OColor(" << color.to_string() << "; Dec " << dec << color.red() << "," << color.green() << "," << color.blue() << ")";
    return os;
}

void OGraphics::print_color_info(const OColor& color, std::ostream& os)
{
    os << "        Color: " << color << endl;
    OGraphics::OColor complementary = color.complementary();
    os << "Complementary: " << complementary << endl;
    OGraphics::OColor darker = color.darker(0.3);
    os << "       Darker: " << darker << endl;
    OGraphics::OColor brighter = color.brighter(0.3);
    os << "     Brighter: " << brighter << endl;
}
