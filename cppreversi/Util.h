#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <cstdint>
#include <string>

using namespace std;

class Util {
public:
    static int bitCountInt1(const uint8_t paramChar);
    static int bitCountInt2(const uint16_t paramShort);
    static int bitCountInt4(const uint32_t paramInt);
    static int bitCountInt8(const uint64_t paramLong);

    static int random(const int idx);

    static std::string pointStr(const uint64_t point, const int boardSize);
};

#endif
