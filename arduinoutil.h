#ifndef INCLUDED_PE32_ARDUINOUTIL_H
#define INCLUDED_PE32_ARDUINOUTIL_H

#include <Print.h>

#include "sensorvalue.h"

// Neat trick to let us do multiple Serial.print() using the << operator:
// Serial << x << " " << y << LF;
template<class T> inline Print &operator<<(Print &obj, T arg) {
    obj.print(arg);
    return obj;
};

// Fix so we can print a floatlike too.
// Example: Serial << some_floatlike; // --> "1.23"
inline Print &operator<<(Print &obj, const floatlike& arg) {
    obj.print((float)arg);
    return obj;
};

// Fix so we can print hexadecimal values.
// Example: Serial << (hextype<uint16_t>)some_int; // --> "C0FFEE"
template<class T> struct hextype {
    hextype(T initial) { value = initial; }
    T value;
};
template<class T> inline Print &operator<<(Print &obj, const hextype<T>& arg) {
    // obj.print("0x");

    // Add one leading 0 so we get an even number of hex digits
    T tmp = arg.value;
    while ((tmp >> 8) > 0) {
        tmp >>= 8;
    }
    if (!(tmp >> 4)) {
        obj.print("0");
    }

    // Do the hex printing
    obj.print(arg.value, HEX);
    return obj;
};

// Fix so we can print float values with the same width.
// Example: Serial << (floattype)some_float; // --> "    1.23"
struct floattype {
    floattype(float initial) { value = initial; }
    float value;
};
inline Print &operator<<(Print &obj, const floattype& arg) {
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wformat-truncation"
    char buf[9];
    // BEWARE: On esp8266/3.1.2 "%8.1F" does shitty formatting with always
    // exponent. On esp32/2.0.14 both 'f' and 'F' work fine.
    if (snprintf(buf, 9, "%8.1f", arg.value) <= 9) {
        obj.print(buf);
    } else {
        obj.print(arg.value);
    }
    return obj;
//#pragma GCC diagnostic pop
};

// vim: set ts=8 sw=4 sts=4 et ai:
#endif //INCLUDED_PE32_ARDUINOUTIL_H
