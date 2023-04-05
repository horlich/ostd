#include "ographics.h"
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace OGraphics;

void OColor::create_value() {
    m_value = m_rgb_array[2].value;  // blue
    unsigned char* arr = reinterpret_cast<unsigned char*>(&m_value);
    arr[1] = m_rgb_array[1].value;   // green
    arr[2] = m_rgb_array[0].value;   // red
}

OColor::OColor(unsigned long rgb) {
    if (rgb > 0xFFFFFF)
        throw range_error("OColor: Value must be in the range from 0 to 0xFFFFFF");
    m_value = rgb;
    unsigned char* arr = reinterpret_cast<unsigned char*>(&rgb);
    for (int i : {2,1,0})
        m_rgb_array[i].value = *arr++;
}

OColor::OColor(unsigned char red, unsigned char green, unsigned char blue) : m_rgb_array{red, green, blue} {
    create_value();
}

bool check_rgb_code__(const string& code) {
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

OColor::OColor(const std::string& rgb_code) {
    if (! check_rgb_code__(rgb_code))
        throw  invalid_argument("malformated color string");
    m_rgb_array[0].value = stoul(rgb_code.substr(1,2), 0, 16);
    m_rgb_array[1].value = stoul(rgb_code.substr(3,2), 0, 16);
    m_rgb_array[2].value = stoul(rgb_code.substr(5,2), 0, 16);
    create_value();
}

OColor OColor::complementary() const {
    return OColor(m_rgb_array[0].complementary(), m_rgb_array[1].complementary(), m_rgb_array[2].complementary());
}

ostream& OGraphics::operator<<(ostream & os, const OColor& color) {
    os << "OColor(Hex " << hex << color.value() << "; Dec " << dec << int(color.red()) << "," << int(color.green()) << "," << int(color.blue()) << ")";
    return os;
}
