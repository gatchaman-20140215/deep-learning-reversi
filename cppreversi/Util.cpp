#include "Util.h"

#include <cstdint>
#include <algorithm>
#ifdef __GNUC__
#include <sys/time.h>
#endif
#ifdef _MSC_VER
#include <sys/timeb.h>
#endif
#include <string>
#include <sstream>

using namespace std;

int Util::bitCountInt1(const uint8_t paramChar) {
    uint8_t count = (paramChar & 0x55) + ((paramChar >> 1) & 0x55);
    count = (count & 0x33) + ((count >> 2) & 0x33);
    return (count & 0x0f) + ((count >> 4) & 0x0f);
}

int Util::bitCountInt2(const uint16_t paramShort) {
    uint16_t count = (paramShort & 0x5555) + ((paramShort >> 1) & 0x5555);
    count = (count & 0x3333) + ((count >> 2) & 0x3333);
    count = (count & 0x0f0f) + ((count >> 4) & 0x0f0f);
    return (count & 0x00ff) + ((count >> 8) & 0x00ff);
}

int Util::bitCountInt4(const uint32_t paramInt) {
    uint32_t count = (paramInt & 0x55555555) + ((paramInt >> 1) & 0x55555555);
    count = (count & 0x33333333) + ((count >> 2) & 0x33333333);
    count = (count & 0x0f0f0f0f) + ((count >> 4) & 0x0f0f0f0f);
    count = (count & 0x00ff00ff) + ((count >> 8) & 0x00ff00ff);
    return (count & 0x0000ffff) + ((count >> 16) & 0x0000ffff);
}

int Util::bitCountInt8(const uint64_t paramLong) {
    uint64_t count = (paramLong & 0x5555555555555555ULL) + ((paramLong >> 1) & 0x5555555555555555ULL);
    count = (count & 0x3333333333333333ULL) + ((count >> 2) & 0x3333333333333333ULL);
    count = (count & 0x0f0f0f0f0f0f0f0fULL) + ((count >> 4) & 0x0f0f0f0f0f0f0f0fULL);
    count = (count & 0x00ff00ff00ff00ffULL) + ((count >> 8) & 0x00ff00ff00ff00ffULL);
    count = (count & 0x0000ffff0000ffffULL) + ((count >> 16) & 0x0000ffff0000ffffULL);
    return (uint32_t)((count & 0x00000000ffffffffULL) + ((count >> 32) & 0x00000000ffffffffULL));
}

int Util::random(const int idx) {
#ifdef __GNUC__
    struct timeval tv;
    gettimeofday(&tv, NULL);
	srand(tv.tv_sec + tv.tv_usec);
#endif
#ifdef __MSC_VER
    _timeb tv;
    _ftime(&tv);
    srand((double)tv.time);
#endif
	return rand() % idx;
}

std::string Util::pointStr(const uint64_t point, const int boardSize) {
    char i = log2(point);
    int x = boardSize - (i % boardSize);
    int y = boardSize - (i / boardSize);
    std::stringstream ss;
    ss << static_cast<char>('a' + x - 1) << static_cast<char>('1' + y - 1);

    return ss.str();
}
