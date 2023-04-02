#pragma once
//#include <Windows.h>
#include <stdint.h>

#define B4TODW_LE(c0, c1, c2, c3) (uint32_t) ((uint32_t) (uint8_t) (c0) | ((uint32_t) (uint8_t) (c1) << 8) | ((uint32_t) (uint8_t) (c2) << 16) | ((uint32_t) (uint8_t) (c3) << 24))
#define B4TODW_BE(c0, c1, c2, c3) (uint32_t) ((uint32_t) (uint8_t) (c3) | ((uint32_t) (uint8_t) (c2) << 8) | ((uint32_t) (uint8_t) (c1) << 16) | ((uint32_t) (uint8_t) (c0) << 24))

#define CODEC_CHARS B4TODW_LE('D', 'I', 'B', ' ')
