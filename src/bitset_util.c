#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitset_util.h"
#include "portability.h"
#include "utilasm.h"


#if defined(IS_X64) || defined(USEAVX)
static uint8_t lengthTable[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4,
    2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4,
    2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6,
    4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5,
    3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6,
    4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};
#endif

#ifdef USEAVX
static uint32_t vecDecodeTable[256][8] ALIGNED(32) = {
    {0, 0, 0, 0, 0, 0, 0, 0}, /* 0x00 (00000000) */
    {1, 0, 0, 0, 0, 0, 0, 0}, /* 0x01 (00000001) */
    {2, 0, 0, 0, 0, 0, 0, 0}, /* 0x02 (00000010) */
    {1, 2, 0, 0, 0, 0, 0, 0}, /* 0x03 (00000011) */
    {3, 0, 0, 0, 0, 0, 0, 0}, /* 0x04 (00000100) */
    {1, 3, 0, 0, 0, 0, 0, 0}, /* 0x05 (00000101) */
    {2, 3, 0, 0, 0, 0, 0, 0}, /* 0x06 (00000110) */
    {1, 2, 3, 0, 0, 0, 0, 0}, /* 0x07 (00000111) */
    {4, 0, 0, 0, 0, 0, 0, 0}, /* 0x08 (00001000) */
    {1, 4, 0, 0, 0, 0, 0, 0}, /* 0x09 (00001001) */
    {2, 4, 0, 0, 0, 0, 0, 0}, /* 0x0A (00001010) */
    {1, 2, 4, 0, 0, 0, 0, 0}, /* 0x0B (00001011) */
    {3, 4, 0, 0, 0, 0, 0, 0}, /* 0x0C (00001100) */
    {1, 3, 4, 0, 0, 0, 0, 0}, /* 0x0D (00001101) */
    {2, 3, 4, 0, 0, 0, 0, 0}, /* 0x0E (00001110) */
    {1, 2, 3, 4, 0, 0, 0, 0}, /* 0x0F (00001111) */
    {5, 0, 0, 0, 0, 0, 0, 0}, /* 0x10 (00010000) */
    {1, 5, 0, 0, 0, 0, 0, 0}, /* 0x11 (00010001) */
    {2, 5, 0, 0, 0, 0, 0, 0}, /* 0x12 (00010010) */
    {1, 2, 5, 0, 0, 0, 0, 0}, /* 0x13 (00010011) */
    {3, 5, 0, 0, 0, 0, 0, 0}, /* 0x14 (00010100) */
    {1, 3, 5, 0, 0, 0, 0, 0}, /* 0x15 (00010101) */
    {2, 3, 5, 0, 0, 0, 0, 0}, /* 0x16 (00010110) */
    {1, 2, 3, 5, 0, 0, 0, 0}, /* 0x17 (00010111) */
    {4, 5, 0, 0, 0, 0, 0, 0}, /* 0x18 (00011000) */
    {1, 4, 5, 0, 0, 0, 0, 0}, /* 0x19 (00011001) */
    {2, 4, 5, 0, 0, 0, 0, 0}, /* 0x1A (00011010) */
    {1, 2, 4, 5, 0, 0, 0, 0}, /* 0x1B (00011011) */
    {3, 4, 5, 0, 0, 0, 0, 0}, /* 0x1C (00011100) */
    {1, 3, 4, 5, 0, 0, 0, 0}, /* 0x1D (00011101) */
    {2, 3, 4, 5, 0, 0, 0, 0}, /* 0x1E (00011110) */
    {1, 2, 3, 4, 5, 0, 0, 0}, /* 0x1F (00011111) */
    {6, 0, 0, 0, 0, 0, 0, 0}, /* 0x20 (00100000) */
    {1, 6, 0, 0, 0, 0, 0, 0}, /* 0x21 (00100001) */
    {2, 6, 0, 0, 0, 0, 0, 0}, /* 0x22 (00100010) */
    {1, 2, 6, 0, 0, 0, 0, 0}, /* 0x23 (00100011) */
    {3, 6, 0, 0, 0, 0, 0, 0}, /* 0x24 (00100100) */
    {1, 3, 6, 0, 0, 0, 0, 0}, /* 0x25 (00100101) */
    {2, 3, 6, 0, 0, 0, 0, 0}, /* 0x26 (00100110) */
    {1, 2, 3, 6, 0, 0, 0, 0}, /* 0x27 (00100111) */
    {4, 6, 0, 0, 0, 0, 0, 0}, /* 0x28 (00101000) */
    {1, 4, 6, 0, 0, 0, 0, 0}, /* 0x29 (00101001) */
    {2, 4, 6, 0, 0, 0, 0, 0}, /* 0x2A (00101010) */
    {1, 2, 4, 6, 0, 0, 0, 0}, /* 0x2B (00101011) */
    {3, 4, 6, 0, 0, 0, 0, 0}, /* 0x2C (00101100) */
    {1, 3, 4, 6, 0, 0, 0, 0}, /* 0x2D (00101101) */
    {2, 3, 4, 6, 0, 0, 0, 0}, /* 0x2E (00101110) */
    {1, 2, 3, 4, 6, 0, 0, 0}, /* 0x2F (00101111) */
    {5, 6, 0, 0, 0, 0, 0, 0}, /* 0x30 (00110000) */
    {1, 5, 6, 0, 0, 0, 0, 0}, /* 0x31 (00110001) */
    {2, 5, 6, 0, 0, 0, 0, 0}, /* 0x32 (00110010) */
    {1, 2, 5, 6, 0, 0, 0, 0}, /* 0x33 (00110011) */
    {3, 5, 6, 0, 0, 0, 0, 0}, /* 0x34 (00110100) */
    {1, 3, 5, 6, 0, 0, 0, 0}, /* 0x35 (00110101) */
    {2, 3, 5, 6, 0, 0, 0, 0}, /* 0x36 (00110110) */
    {1, 2, 3, 5, 6, 0, 0, 0}, /* 0x37 (00110111) */
    {4, 5, 6, 0, 0, 0, 0, 0}, /* 0x38 (00111000) */
    {1, 4, 5, 6, 0, 0, 0, 0}, /* 0x39 (00111001) */
    {2, 4, 5, 6, 0, 0, 0, 0}, /* 0x3A (00111010) */
    {1, 2, 4, 5, 6, 0, 0, 0}, /* 0x3B (00111011) */
    {3, 4, 5, 6, 0, 0, 0, 0}, /* 0x3C (00111100) */
    {1, 3, 4, 5, 6, 0, 0, 0}, /* 0x3D (00111101) */
    {2, 3, 4, 5, 6, 0, 0, 0}, /* 0x3E (00111110) */
    {1, 2, 3, 4, 5, 6, 0, 0}, /* 0x3F (00111111) */
    {7, 0, 0, 0, 0, 0, 0, 0}, /* 0x40 (01000000) */
    {1, 7, 0, 0, 0, 0, 0, 0}, /* 0x41 (01000001) */
    {2, 7, 0, 0, 0, 0, 0, 0}, /* 0x42 (01000010) */
    {1, 2, 7, 0, 0, 0, 0, 0}, /* 0x43 (01000011) */
    {3, 7, 0, 0, 0, 0, 0, 0}, /* 0x44 (01000100) */
    {1, 3, 7, 0, 0, 0, 0, 0}, /* 0x45 (01000101) */
    {2, 3, 7, 0, 0, 0, 0, 0}, /* 0x46 (01000110) */
    {1, 2, 3, 7, 0, 0, 0, 0}, /* 0x47 (01000111) */
    {4, 7, 0, 0, 0, 0, 0, 0}, /* 0x48 (01001000) */
    {1, 4, 7, 0, 0, 0, 0, 0}, /* 0x49 (01001001) */
    {2, 4, 7, 0, 0, 0, 0, 0}, /* 0x4A (01001010) */
    {1, 2, 4, 7, 0, 0, 0, 0}, /* 0x4B (01001011) */
    {3, 4, 7, 0, 0, 0, 0, 0}, /* 0x4C (01001100) */
    {1, 3, 4, 7, 0, 0, 0, 0}, /* 0x4D (01001101) */
    {2, 3, 4, 7, 0, 0, 0, 0}, /* 0x4E (01001110) */
    {1, 2, 3, 4, 7, 0, 0, 0}, /* 0x4F (01001111) */
    {5, 7, 0, 0, 0, 0, 0, 0}, /* 0x50 (01010000) */
    {1, 5, 7, 0, 0, 0, 0, 0}, /* 0x51 (01010001) */
    {2, 5, 7, 0, 0, 0, 0, 0}, /* 0x52 (01010010) */
    {1, 2, 5, 7, 0, 0, 0, 0}, /* 0x53 (01010011) */
    {3, 5, 7, 0, 0, 0, 0, 0}, /* 0x54 (01010100) */
    {1, 3, 5, 7, 0, 0, 0, 0}, /* 0x55 (01010101) */
    {2, 3, 5, 7, 0, 0, 0, 0}, /* 0x56 (01010110) */
    {1, 2, 3, 5, 7, 0, 0, 0}, /* 0x57 (01010111) */
    {4, 5, 7, 0, 0, 0, 0, 0}, /* 0x58 (01011000) */
    {1, 4, 5, 7, 0, 0, 0, 0}, /* 0x59 (01011001) */
    {2, 4, 5, 7, 0, 0, 0, 0}, /* 0x5A (01011010) */
    {1, 2, 4, 5, 7, 0, 0, 0}, /* 0x5B (01011011) */
    {3, 4, 5, 7, 0, 0, 0, 0}, /* 0x5C (01011100) */
    {1, 3, 4, 5, 7, 0, 0, 0}, /* 0x5D (01011101) */
    {2, 3, 4, 5, 7, 0, 0, 0}, /* 0x5E (01011110) */
    {1, 2, 3, 4, 5, 7, 0, 0}, /* 0x5F (01011111) */
    {6, 7, 0, 0, 0, 0, 0, 0}, /* 0x60 (01100000) */
    {1, 6, 7, 0, 0, 0, 0, 0}, /* 0x61 (01100001) */
    {2, 6, 7, 0, 0, 0, 0, 0}, /* 0x62 (01100010) */
    {1, 2, 6, 7, 0, 0, 0, 0}, /* 0x63 (01100011) */
    {3, 6, 7, 0, 0, 0, 0, 0}, /* 0x64 (01100100) */
    {1, 3, 6, 7, 0, 0, 0, 0}, /* 0x65 (01100101) */
    {2, 3, 6, 7, 0, 0, 0, 0}, /* 0x66 (01100110) */
    {1, 2, 3, 6, 7, 0, 0, 0}, /* 0x67 (01100111) */
    {4, 6, 7, 0, 0, 0, 0, 0}, /* 0x68 (01101000) */
    {1, 4, 6, 7, 0, 0, 0, 0}, /* 0x69 (01101001) */
    {2, 4, 6, 7, 0, 0, 0, 0}, /* 0x6A (01101010) */
    {1, 2, 4, 6, 7, 0, 0, 0}, /* 0x6B (01101011) */
    {3, 4, 6, 7, 0, 0, 0, 0}, /* 0x6C (01101100) */
    {1, 3, 4, 6, 7, 0, 0, 0}, /* 0x6D (01101101) */
    {2, 3, 4, 6, 7, 0, 0, 0}, /* 0x6E (01101110) */
    {1, 2, 3, 4, 6, 7, 0, 0}, /* 0x6F (01101111) */
    {5, 6, 7, 0, 0, 0, 0, 0}, /* 0x70 (01110000) */
    {1, 5, 6, 7, 0, 0, 0, 0}, /* 0x71 (01110001) */
    {2, 5, 6, 7, 0, 0, 0, 0}, /* 0x72 (01110010) */
    {1, 2, 5, 6, 7, 0, 0, 0}, /* 0x73 (01110011) */
    {3, 5, 6, 7, 0, 0, 0, 0}, /* 0x74 (01110100) */
    {1, 3, 5, 6, 7, 0, 0, 0}, /* 0x75 (01110101) */
    {2, 3, 5, 6, 7, 0, 0, 0}, /* 0x76 (01110110) */
    {1, 2, 3, 5, 6, 7, 0, 0}, /* 0x77 (01110111) */
    {4, 5, 6, 7, 0, 0, 0, 0}, /* 0x78 (01111000) */
    {1, 4, 5, 6, 7, 0, 0, 0}, /* 0x79 (01111001) */
    {2, 4, 5, 6, 7, 0, 0, 0}, /* 0x7A (01111010) */
    {1, 2, 4, 5, 6, 7, 0, 0}, /* 0x7B (01111011) */
    {3, 4, 5, 6, 7, 0, 0, 0}, /* 0x7C (01111100) */
    {1, 3, 4, 5, 6, 7, 0, 0}, /* 0x7D (01111101) */
    {2, 3, 4, 5, 6, 7, 0, 0}, /* 0x7E (01111110) */
    {1, 2, 3, 4, 5, 6, 7, 0}, /* 0x7F (01111111) */
    {8, 0, 0, 0, 0, 0, 0, 0}, /* 0x80 (10000000) */
    {1, 8, 0, 0, 0, 0, 0, 0}, /* 0x81 (10000001) */
    {2, 8, 0, 0, 0, 0, 0, 0}, /* 0x82 (10000010) */
    {1, 2, 8, 0, 0, 0, 0, 0}, /* 0x83 (10000011) */
    {3, 8, 0, 0, 0, 0, 0, 0}, /* 0x84 (10000100) */
    {1, 3, 8, 0, 0, 0, 0, 0}, /* 0x85 (10000101) */
    {2, 3, 8, 0, 0, 0, 0, 0}, /* 0x86 (10000110) */
    {1, 2, 3, 8, 0, 0, 0, 0}, /* 0x87 (10000111) */
    {4, 8, 0, 0, 0, 0, 0, 0}, /* 0x88 (10001000) */
    {1, 4, 8, 0, 0, 0, 0, 0}, /* 0x89 (10001001) */
    {2, 4, 8, 0, 0, 0, 0, 0}, /* 0x8A (10001010) */
    {1, 2, 4, 8, 0, 0, 0, 0}, /* 0x8B (10001011) */
    {3, 4, 8, 0, 0, 0, 0, 0}, /* 0x8C (10001100) */
    {1, 3, 4, 8, 0, 0, 0, 0}, /* 0x8D (10001101) */
    {2, 3, 4, 8, 0, 0, 0, 0}, /* 0x8E (10001110) */
    {1, 2, 3, 4, 8, 0, 0, 0}, /* 0x8F (10001111) */
    {5, 8, 0, 0, 0, 0, 0, 0}, /* 0x90 (10010000) */
    {1, 5, 8, 0, 0, 0, 0, 0}, /* 0x91 (10010001) */
    {2, 5, 8, 0, 0, 0, 0, 0}, /* 0x92 (10010010) */
    {1, 2, 5, 8, 0, 0, 0, 0}, /* 0x93 (10010011) */
    {3, 5, 8, 0, 0, 0, 0, 0}, /* 0x94 (10010100) */
    {1, 3, 5, 8, 0, 0, 0, 0}, /* 0x95 (10010101) */
    {2, 3, 5, 8, 0, 0, 0, 0}, /* 0x96 (10010110) */
    {1, 2, 3, 5, 8, 0, 0, 0}, /* 0x97 (10010111) */
    {4, 5, 8, 0, 0, 0, 0, 0}, /* 0x98 (10011000) */
    {1, 4, 5, 8, 0, 0, 0, 0}, /* 0x99 (10011001) */
    {2, 4, 5, 8, 0, 0, 0, 0}, /* 0x9A (10011010) */
    {1, 2, 4, 5, 8, 0, 0, 0}, /* 0x9B (10011011) */
    {3, 4, 5, 8, 0, 0, 0, 0}, /* 0x9C (10011100) */
    {1, 3, 4, 5, 8, 0, 0, 0}, /* 0x9D (10011101) */
    {2, 3, 4, 5, 8, 0, 0, 0}, /* 0x9E (10011110) */
    {1, 2, 3, 4, 5, 8, 0, 0}, /* 0x9F (10011111) */
    {6, 8, 0, 0, 0, 0, 0, 0}, /* 0xA0 (10100000) */
    {1, 6, 8, 0, 0, 0, 0, 0}, /* 0xA1 (10100001) */
    {2, 6, 8, 0, 0, 0, 0, 0}, /* 0xA2 (10100010) */
    {1, 2, 6, 8, 0, 0, 0, 0}, /* 0xA3 (10100011) */
    {3, 6, 8, 0, 0, 0, 0, 0}, /* 0xA4 (10100100) */
    {1, 3, 6, 8, 0, 0, 0, 0}, /* 0xA5 (10100101) */
    {2, 3, 6, 8, 0, 0, 0, 0}, /* 0xA6 (10100110) */
    {1, 2, 3, 6, 8, 0, 0, 0}, /* 0xA7 (10100111) */
    {4, 6, 8, 0, 0, 0, 0, 0}, /* 0xA8 (10101000) */
    {1, 4, 6, 8, 0, 0, 0, 0}, /* 0xA9 (10101001) */
    {2, 4, 6, 8, 0, 0, 0, 0}, /* 0xAA (10101010) */
    {1, 2, 4, 6, 8, 0, 0, 0}, /* 0xAB (10101011) */
    {3, 4, 6, 8, 0, 0, 0, 0}, /* 0xAC (10101100) */
    {1, 3, 4, 6, 8, 0, 0, 0}, /* 0xAD (10101101) */
    {2, 3, 4, 6, 8, 0, 0, 0}, /* 0xAE (10101110) */
    {1, 2, 3, 4, 6, 8, 0, 0}, /* 0xAF (10101111) */
    {5, 6, 8, 0, 0, 0, 0, 0}, /* 0xB0 (10110000) */
    {1, 5, 6, 8, 0, 0, 0, 0}, /* 0xB1 (10110001) */
    {2, 5, 6, 8, 0, 0, 0, 0}, /* 0xB2 (10110010) */
    {1, 2, 5, 6, 8, 0, 0, 0}, /* 0xB3 (10110011) */
    {3, 5, 6, 8, 0, 0, 0, 0}, /* 0xB4 (10110100) */
    {1, 3, 5, 6, 8, 0, 0, 0}, /* 0xB5 (10110101) */
    {2, 3, 5, 6, 8, 0, 0, 0}, /* 0xB6 (10110110) */
    {1, 2, 3, 5, 6, 8, 0, 0}, /* 0xB7 (10110111) */
    {4, 5, 6, 8, 0, 0, 0, 0}, /* 0xB8 (10111000) */
    {1, 4, 5, 6, 8, 0, 0, 0}, /* 0xB9 (10111001) */
    {2, 4, 5, 6, 8, 0, 0, 0}, /* 0xBA (10111010) */
    {1, 2, 4, 5, 6, 8, 0, 0}, /* 0xBB (10111011) */
    {3, 4, 5, 6, 8, 0, 0, 0}, /* 0xBC (10111100) */
    {1, 3, 4, 5, 6, 8, 0, 0}, /* 0xBD (10111101) */
    {2, 3, 4, 5, 6, 8, 0, 0}, /* 0xBE (10111110) */
    {1, 2, 3, 4, 5, 6, 8, 0}, /* 0xBF (10111111) */
    {7, 8, 0, 0, 0, 0, 0, 0}, /* 0xC0 (11000000) */
    {1, 7, 8, 0, 0, 0, 0, 0}, /* 0xC1 (11000001) */
    {2, 7, 8, 0, 0, 0, 0, 0}, /* 0xC2 (11000010) */
    {1, 2, 7, 8, 0, 0, 0, 0}, /* 0xC3 (11000011) */
    {3, 7, 8, 0, 0, 0, 0, 0}, /* 0xC4 (11000100) */
    {1, 3, 7, 8, 0, 0, 0, 0}, /* 0xC5 (11000101) */
    {2, 3, 7, 8, 0, 0, 0, 0}, /* 0xC6 (11000110) */
    {1, 2, 3, 7, 8, 0, 0, 0}, /* 0xC7 (11000111) */
    {4, 7, 8, 0, 0, 0, 0, 0}, /* 0xC8 (11001000) */
    {1, 4, 7, 8, 0, 0, 0, 0}, /* 0xC9 (11001001) */
    {2, 4, 7, 8, 0, 0, 0, 0}, /* 0xCA (11001010) */
    {1, 2, 4, 7, 8, 0, 0, 0}, /* 0xCB (11001011) */
    {3, 4, 7, 8, 0, 0, 0, 0}, /* 0xCC (11001100) */
    {1, 3, 4, 7, 8, 0, 0, 0}, /* 0xCD (11001101) */
    {2, 3, 4, 7, 8, 0, 0, 0}, /* 0xCE (11001110) */
    {1, 2, 3, 4, 7, 8, 0, 0}, /* 0xCF (11001111) */
    {5, 7, 8, 0, 0, 0, 0, 0}, /* 0xD0 (11010000) */
    {1, 5, 7, 8, 0, 0, 0, 0}, /* 0xD1 (11010001) */
    {2, 5, 7, 8, 0, 0, 0, 0}, /* 0xD2 (11010010) */
    {1, 2, 5, 7, 8, 0, 0, 0}, /* 0xD3 (11010011) */
    {3, 5, 7, 8, 0, 0, 0, 0}, /* 0xD4 (11010100) */
    {1, 3, 5, 7, 8, 0, 0, 0}, /* 0xD5 (11010101) */
    {2, 3, 5, 7, 8, 0, 0, 0}, /* 0xD6 (11010110) */
    {1, 2, 3, 5, 7, 8, 0, 0}, /* 0xD7 (11010111) */
    {4, 5, 7, 8, 0, 0, 0, 0}, /* 0xD8 (11011000) */
    {1, 4, 5, 7, 8, 0, 0, 0}, /* 0xD9 (11011001) */
    {2, 4, 5, 7, 8, 0, 0, 0}, /* 0xDA (11011010) */
    {1, 2, 4, 5, 7, 8, 0, 0}, /* 0xDB (11011011) */
    {3, 4, 5, 7, 8, 0, 0, 0}, /* 0xDC (11011100) */
    {1, 3, 4, 5, 7, 8, 0, 0}, /* 0xDD (11011101) */
    {2, 3, 4, 5, 7, 8, 0, 0}, /* 0xDE (11011110) */
    {1, 2, 3, 4, 5, 7, 8, 0}, /* 0xDF (11011111) */
    {6, 7, 8, 0, 0, 0, 0, 0}, /* 0xE0 (11100000) */
    {1, 6, 7, 8, 0, 0, 0, 0}, /* 0xE1 (11100001) */
    {2, 6, 7, 8, 0, 0, 0, 0}, /* 0xE2 (11100010) */
    {1, 2, 6, 7, 8, 0, 0, 0}, /* 0xE3 (11100011) */
    {3, 6, 7, 8, 0, 0, 0, 0}, /* 0xE4 (11100100) */
    {1, 3, 6, 7, 8, 0, 0, 0}, /* 0xE5 (11100101) */
    {2, 3, 6, 7, 8, 0, 0, 0}, /* 0xE6 (11100110) */
    {1, 2, 3, 6, 7, 8, 0, 0}, /* 0xE7 (11100111) */
    {4, 6, 7, 8, 0, 0, 0, 0}, /* 0xE8 (11101000) */
    {1, 4, 6, 7, 8, 0, 0, 0}, /* 0xE9 (11101001) */
    {2, 4, 6, 7, 8, 0, 0, 0}, /* 0xEA (11101010) */
    {1, 2, 4, 6, 7, 8, 0, 0}, /* 0xEB (11101011) */
    {3, 4, 6, 7, 8, 0, 0, 0}, /* 0xEC (11101100) */
    {1, 3, 4, 6, 7, 8, 0, 0}, /* 0xED (11101101) */
    {2, 3, 4, 6, 7, 8, 0, 0}, /* 0xEE (11101110) */
    {1, 2, 3, 4, 6, 7, 8, 0}, /* 0xEF (11101111) */
    {5, 6, 7, 8, 0, 0, 0, 0}, /* 0xF0 (11110000) */
    {1, 5, 6, 7, 8, 0, 0, 0}, /* 0xF1 (11110001) */
    {2, 5, 6, 7, 8, 0, 0, 0}, /* 0xF2 (11110010) */
    {1, 2, 5, 6, 7, 8, 0, 0}, /* 0xF3 (11110011) */
    {3, 5, 6, 7, 8, 0, 0, 0}, /* 0xF4 (11110100) */
    {1, 3, 5, 6, 7, 8, 0, 0}, /* 0xF5 (11110101) */
    {2, 3, 5, 6, 7, 8, 0, 0}, /* 0xF6 (11110110) */
    {1, 2, 3, 5, 6, 7, 8, 0}, /* 0xF7 (11110111) */
    {4, 5, 6, 7, 8, 0, 0, 0}, /* 0xF8 (11111000) */
    {1, 4, 5, 6, 7, 8, 0, 0}, /* 0xF9 (11111001) */
    {2, 4, 5, 6, 7, 8, 0, 0}, /* 0xFA (11111010) */
    {1, 2, 4, 5, 6, 7, 8, 0}, /* 0xFB (11111011) */
    {3, 4, 5, 6, 7, 8, 0, 0}, /* 0xFC (11111100) */
    {1, 3, 4, 5, 6, 7, 8, 0}, /* 0xFD (11111101) */
    {2, 3, 4, 5, 6, 7, 8, 0}, /* 0xFE (11111110) */
    {1, 2, 3, 4, 5, 6, 7, 8}  /* 0xFF (11111111) */
};

#endif // #ifdef USEAVX

#ifdef IS_X64
// same as vecDecodeTable but in 16 bits
static uint16_t vecDecodeTable_uint16[256][8] ALIGNED(32) = {
    {0, 0, 0, 0, 0, 0, 0, 0}, /* 0x00 (00000000) */
    {1, 0, 0, 0, 0, 0, 0, 0}, /* 0x01 (00000001) */
    {2, 0, 0, 0, 0, 0, 0, 0}, /* 0x02 (00000010) */
    {1, 2, 0, 0, 0, 0, 0, 0}, /* 0x03 (00000011) */
    {3, 0, 0, 0, 0, 0, 0, 0}, /* 0x04 (00000100) */
    {1, 3, 0, 0, 0, 0, 0, 0}, /* 0x05 (00000101) */
    {2, 3, 0, 0, 0, 0, 0, 0}, /* 0x06 (00000110) */
    {1, 2, 3, 0, 0, 0, 0, 0}, /* 0x07 (00000111) */
    {4, 0, 0, 0, 0, 0, 0, 0}, /* 0x08 (00001000) */
    {1, 4, 0, 0, 0, 0, 0, 0}, /* 0x09 (00001001) */
    {2, 4, 0, 0, 0, 0, 0, 0}, /* 0x0A (00001010) */
    {1, 2, 4, 0, 0, 0, 0, 0}, /* 0x0B (00001011) */
    {3, 4, 0, 0, 0, 0, 0, 0}, /* 0x0C (00001100) */
    {1, 3, 4, 0, 0, 0, 0, 0}, /* 0x0D (00001101) */
    {2, 3, 4, 0, 0, 0, 0, 0}, /* 0x0E (00001110) */
    {1, 2, 3, 4, 0, 0, 0, 0}, /* 0x0F (00001111) */
    {5, 0, 0, 0, 0, 0, 0, 0}, /* 0x10 (00010000) */
    {1, 5, 0, 0, 0, 0, 0, 0}, /* 0x11 (00010001) */
    {2, 5, 0, 0, 0, 0, 0, 0}, /* 0x12 (00010010) */
    {1, 2, 5, 0, 0, 0, 0, 0}, /* 0x13 (00010011) */
    {3, 5, 0, 0, 0, 0, 0, 0}, /* 0x14 (00010100) */
    {1, 3, 5, 0, 0, 0, 0, 0}, /* 0x15 (00010101) */
    {2, 3, 5, 0, 0, 0, 0, 0}, /* 0x16 (00010110) */
    {1, 2, 3, 5, 0, 0, 0, 0}, /* 0x17 (00010111) */
    {4, 5, 0, 0, 0, 0, 0, 0}, /* 0x18 (00011000) */
    {1, 4, 5, 0, 0, 0, 0, 0}, /* 0x19 (00011001) */
    {2, 4, 5, 0, 0, 0, 0, 0}, /* 0x1A (00011010) */
    {1, 2, 4, 5, 0, 0, 0, 0}, /* 0x1B (00011011) */
    {3, 4, 5, 0, 0, 0, 0, 0}, /* 0x1C (00011100) */
    {1, 3, 4, 5, 0, 0, 0, 0}, /* 0x1D (00011101) */
    {2, 3, 4, 5, 0, 0, 0, 0}, /* 0x1E (00011110) */
    {1, 2, 3, 4, 5, 0, 0, 0}, /* 0x1F (00011111) */
    {6, 0, 0, 0, 0, 0, 0, 0}, /* 0x20 (00100000) */
    {1, 6, 0, 0, 0, 0, 0, 0}, /* 0x21 (00100001) */
    {2, 6, 0, 0, 0, 0, 0, 0}, /* 0x22 (00100010) */
    {1, 2, 6, 0, 0, 0, 0, 0}, /* 0x23 (00100011) */
    {3, 6, 0, 0, 0, 0, 0, 0}, /* 0x24 (00100100) */
    {1, 3, 6, 0, 0, 0, 0, 0}, /* 0x25 (00100101) */
    {2, 3, 6, 0, 0, 0, 0, 0}, /* 0x26 (00100110) */
    {1, 2, 3, 6, 0, 0, 0, 0}, /* 0x27 (00100111) */
    {4, 6, 0, 0, 0, 0, 0, 0}, /* 0x28 (00101000) */
    {1, 4, 6, 0, 0, 0, 0, 0}, /* 0x29 (00101001) */
    {2, 4, 6, 0, 0, 0, 0, 0}, /* 0x2A (00101010) */
    {1, 2, 4, 6, 0, 0, 0, 0}, /* 0x2B (00101011) */
    {3, 4, 6, 0, 0, 0, 0, 0}, /* 0x2C (00101100) */
    {1, 3, 4, 6, 0, 0, 0, 0}, /* 0x2D (00101101) */
    {2, 3, 4, 6, 0, 0, 0, 0}, /* 0x2E (00101110) */
    {1, 2, 3, 4, 6, 0, 0, 0}, /* 0x2F (00101111) */
    {5, 6, 0, 0, 0, 0, 0, 0}, /* 0x30 (00110000) */
    {1, 5, 6, 0, 0, 0, 0, 0}, /* 0x31 (00110001) */
    {2, 5, 6, 0, 0, 0, 0, 0}, /* 0x32 (00110010) */
    {1, 2, 5, 6, 0, 0, 0, 0}, /* 0x33 (00110011) */
    {3, 5, 6, 0, 0, 0, 0, 0}, /* 0x34 (00110100) */
    {1, 3, 5, 6, 0, 0, 0, 0}, /* 0x35 (00110101) */
    {2, 3, 5, 6, 0, 0, 0, 0}, /* 0x36 (00110110) */
    {1, 2, 3, 5, 6, 0, 0, 0}, /* 0x37 (00110111) */
    {4, 5, 6, 0, 0, 0, 0, 0}, /* 0x38 (00111000) */
    {1, 4, 5, 6, 0, 0, 0, 0}, /* 0x39 (00111001) */
    {2, 4, 5, 6, 0, 0, 0, 0}, /* 0x3A (00111010) */
    {1, 2, 4, 5, 6, 0, 0, 0}, /* 0x3B (00111011) */
    {3, 4, 5, 6, 0, 0, 0, 0}, /* 0x3C (00111100) */
    {1, 3, 4, 5, 6, 0, 0, 0}, /* 0x3D (00111101) */
    {2, 3, 4, 5, 6, 0, 0, 0}, /* 0x3E (00111110) */
    {1, 2, 3, 4, 5, 6, 0, 0}, /* 0x3F (00111111) */
    {7, 0, 0, 0, 0, 0, 0, 0}, /* 0x40 (01000000) */
    {1, 7, 0, 0, 0, 0, 0, 0}, /* 0x41 (01000001) */
    {2, 7, 0, 0, 0, 0, 0, 0}, /* 0x42 (01000010) */
    {1, 2, 7, 0, 0, 0, 0, 0}, /* 0x43 (01000011) */
    {3, 7, 0, 0, 0, 0, 0, 0}, /* 0x44 (01000100) */
    {1, 3, 7, 0, 0, 0, 0, 0}, /* 0x45 (01000101) */
    {2, 3, 7, 0, 0, 0, 0, 0}, /* 0x46 (01000110) */
    {1, 2, 3, 7, 0, 0, 0, 0}, /* 0x47 (01000111) */
    {4, 7, 0, 0, 0, 0, 0, 0}, /* 0x48 (01001000) */
    {1, 4, 7, 0, 0, 0, 0, 0}, /* 0x49 (01001001) */
    {2, 4, 7, 0, 0, 0, 0, 0}, /* 0x4A (01001010) */
    {1, 2, 4, 7, 0, 0, 0, 0}, /* 0x4B (01001011) */
    {3, 4, 7, 0, 0, 0, 0, 0}, /* 0x4C (01001100) */
    {1, 3, 4, 7, 0, 0, 0, 0}, /* 0x4D (01001101) */
    {2, 3, 4, 7, 0, 0, 0, 0}, /* 0x4E (01001110) */
    {1, 2, 3, 4, 7, 0, 0, 0}, /* 0x4F (01001111) */
    {5, 7, 0, 0, 0, 0, 0, 0}, /* 0x50 (01010000) */
    {1, 5, 7, 0, 0, 0, 0, 0}, /* 0x51 (01010001) */
    {2, 5, 7, 0, 0, 0, 0, 0}, /* 0x52 (01010010) */
    {1, 2, 5, 7, 0, 0, 0, 0}, /* 0x53 (01010011) */
    {3, 5, 7, 0, 0, 0, 0, 0}, /* 0x54 (01010100) */
    {1, 3, 5, 7, 0, 0, 0, 0}, /* 0x55 (01010101) */
    {2, 3, 5, 7, 0, 0, 0, 0}, /* 0x56 (01010110) */
    {1, 2, 3, 5, 7, 0, 0, 0}, /* 0x57 (01010111) */
    {4, 5, 7, 0, 0, 0, 0, 0}, /* 0x58 (01011000) */
    {1, 4, 5, 7, 0, 0, 0, 0}, /* 0x59 (01011001) */
    {2, 4, 5, 7, 0, 0, 0, 0}, /* 0x5A (01011010) */
    {1, 2, 4, 5, 7, 0, 0, 0}, /* 0x5B (01011011) */
    {3, 4, 5, 7, 0, 0, 0, 0}, /* 0x5C (01011100) */
    {1, 3, 4, 5, 7, 0, 0, 0}, /* 0x5D (01011101) */
    {2, 3, 4, 5, 7, 0, 0, 0}, /* 0x5E (01011110) */
    {1, 2, 3, 4, 5, 7, 0, 0}, /* 0x5F (01011111) */
    {6, 7, 0, 0, 0, 0, 0, 0}, /* 0x60 (01100000) */
    {1, 6, 7, 0, 0, 0, 0, 0}, /* 0x61 (01100001) */
    {2, 6, 7, 0, 0, 0, 0, 0}, /* 0x62 (01100010) */
    {1, 2, 6, 7, 0, 0, 0, 0}, /* 0x63 (01100011) */
    {3, 6, 7, 0, 0, 0, 0, 0}, /* 0x64 (01100100) */
    {1, 3, 6, 7, 0, 0, 0, 0}, /* 0x65 (01100101) */
    {2, 3, 6, 7, 0, 0, 0, 0}, /* 0x66 (01100110) */
    {1, 2, 3, 6, 7, 0, 0, 0}, /* 0x67 (01100111) */
    {4, 6, 7, 0, 0, 0, 0, 0}, /* 0x68 (01101000) */
    {1, 4, 6, 7, 0, 0, 0, 0}, /* 0x69 (01101001) */
    {2, 4, 6, 7, 0, 0, 0, 0}, /* 0x6A (01101010) */
    {1, 2, 4, 6, 7, 0, 0, 0}, /* 0x6B (01101011) */
    {3, 4, 6, 7, 0, 0, 0, 0}, /* 0x6C (01101100) */
    {1, 3, 4, 6, 7, 0, 0, 0}, /* 0x6D (01101101) */
    {2, 3, 4, 6, 7, 0, 0, 0}, /* 0x6E (01101110) */
    {1, 2, 3, 4, 6, 7, 0, 0}, /* 0x6F (01101111) */
    {5, 6, 7, 0, 0, 0, 0, 0}, /* 0x70 (01110000) */
    {1, 5, 6, 7, 0, 0, 0, 0}, /* 0x71 (01110001) */
    {2, 5, 6, 7, 0, 0, 0, 0}, /* 0x72 (01110010) */
    {1, 2, 5, 6, 7, 0, 0, 0}, /* 0x73 (01110011) */
    {3, 5, 6, 7, 0, 0, 0, 0}, /* 0x74 (01110100) */
    {1, 3, 5, 6, 7, 0, 0, 0}, /* 0x75 (01110101) */
    {2, 3, 5, 6, 7, 0, 0, 0}, /* 0x76 (01110110) */
    {1, 2, 3, 5, 6, 7, 0, 0}, /* 0x77 (01110111) */
    {4, 5, 6, 7, 0, 0, 0, 0}, /* 0x78 (01111000) */
    {1, 4, 5, 6, 7, 0, 0, 0}, /* 0x79 (01111001) */
    {2, 4, 5, 6, 7, 0, 0, 0}, /* 0x7A (01111010) */
    {1, 2, 4, 5, 6, 7, 0, 0}, /* 0x7B (01111011) */
    {3, 4, 5, 6, 7, 0, 0, 0}, /* 0x7C (01111100) */
    {1, 3, 4, 5, 6, 7, 0, 0}, /* 0x7D (01111101) */
    {2, 3, 4, 5, 6, 7, 0, 0}, /* 0x7E (01111110) */
    {1, 2, 3, 4, 5, 6, 7, 0}, /* 0x7F (01111111) */
    {8, 0, 0, 0, 0, 0, 0, 0}, /* 0x80 (10000000) */
    {1, 8, 0, 0, 0, 0, 0, 0}, /* 0x81 (10000001) */
    {2, 8, 0, 0, 0, 0, 0, 0}, /* 0x82 (10000010) */
    {1, 2, 8, 0, 0, 0, 0, 0}, /* 0x83 (10000011) */
    {3, 8, 0, 0, 0, 0, 0, 0}, /* 0x84 (10000100) */
    {1, 3, 8, 0, 0, 0, 0, 0}, /* 0x85 (10000101) */
    {2, 3, 8, 0, 0, 0, 0, 0}, /* 0x86 (10000110) */
    {1, 2, 3, 8, 0, 0, 0, 0}, /* 0x87 (10000111) */
    {4, 8, 0, 0, 0, 0, 0, 0}, /* 0x88 (10001000) */
    {1, 4, 8, 0, 0, 0, 0, 0}, /* 0x89 (10001001) */
    {2, 4, 8, 0, 0, 0, 0, 0}, /* 0x8A (10001010) */
    {1, 2, 4, 8, 0, 0, 0, 0}, /* 0x8B (10001011) */
    {3, 4, 8, 0, 0, 0, 0, 0}, /* 0x8C (10001100) */
    {1, 3, 4, 8, 0, 0, 0, 0}, /* 0x8D (10001101) */
    {2, 3, 4, 8, 0, 0, 0, 0}, /* 0x8E (10001110) */
    {1, 2, 3, 4, 8, 0, 0, 0}, /* 0x8F (10001111) */
    {5, 8, 0, 0, 0, 0, 0, 0}, /* 0x90 (10010000) */
    {1, 5, 8, 0, 0, 0, 0, 0}, /* 0x91 (10010001) */
    {2, 5, 8, 0, 0, 0, 0, 0}, /* 0x92 (10010010) */
    {1, 2, 5, 8, 0, 0, 0, 0}, /* 0x93 (10010011) */
    {3, 5, 8, 0, 0, 0, 0, 0}, /* 0x94 (10010100) */
    {1, 3, 5, 8, 0, 0, 0, 0}, /* 0x95 (10010101) */
    {2, 3, 5, 8, 0, 0, 0, 0}, /* 0x96 (10010110) */
    {1, 2, 3, 5, 8, 0, 0, 0}, /* 0x97 (10010111) */
    {4, 5, 8, 0, 0, 0, 0, 0}, /* 0x98 (10011000) */
    {1, 4, 5, 8, 0, 0, 0, 0}, /* 0x99 (10011001) */
    {2, 4, 5, 8, 0, 0, 0, 0}, /* 0x9A (10011010) */
    {1, 2, 4, 5, 8, 0, 0, 0}, /* 0x9B (10011011) */
    {3, 4, 5, 8, 0, 0, 0, 0}, /* 0x9C (10011100) */
    {1, 3, 4, 5, 8, 0, 0, 0}, /* 0x9D (10011101) */
    {2, 3, 4, 5, 8, 0, 0, 0}, /* 0x9E (10011110) */
    {1, 2, 3, 4, 5, 8, 0, 0}, /* 0x9F (10011111) */
    {6, 8, 0, 0, 0, 0, 0, 0}, /* 0xA0 (10100000) */
    {1, 6, 8, 0, 0, 0, 0, 0}, /* 0xA1 (10100001) */
    {2, 6, 8, 0, 0, 0, 0, 0}, /* 0xA2 (10100010) */
    {1, 2, 6, 8, 0, 0, 0, 0}, /* 0xA3 (10100011) */
    {3, 6, 8, 0, 0, 0, 0, 0}, /* 0xA4 (10100100) */
    {1, 3, 6, 8, 0, 0, 0, 0}, /* 0xA5 (10100101) */
    {2, 3, 6, 8, 0, 0, 0, 0}, /* 0xA6 (10100110) */
    {1, 2, 3, 6, 8, 0, 0, 0}, /* 0xA7 (10100111) */
    {4, 6, 8, 0, 0, 0, 0, 0}, /* 0xA8 (10101000) */
    {1, 4, 6, 8, 0, 0, 0, 0}, /* 0xA9 (10101001) */
    {2, 4, 6, 8, 0, 0, 0, 0}, /* 0xAA (10101010) */
    {1, 2, 4, 6, 8, 0, 0, 0}, /* 0xAB (10101011) */
    {3, 4, 6, 8, 0, 0, 0, 0}, /* 0xAC (10101100) */
    {1, 3, 4, 6, 8, 0, 0, 0}, /* 0xAD (10101101) */
    {2, 3, 4, 6, 8, 0, 0, 0}, /* 0xAE (10101110) */
    {1, 2, 3, 4, 6, 8, 0, 0}, /* 0xAF (10101111) */
    {5, 6, 8, 0, 0, 0, 0, 0}, /* 0xB0 (10110000) */
    {1, 5, 6, 8, 0, 0, 0, 0}, /* 0xB1 (10110001) */
    {2, 5, 6, 8, 0, 0, 0, 0}, /* 0xB2 (10110010) */
    {1, 2, 5, 6, 8, 0, 0, 0}, /* 0xB3 (10110011) */
    {3, 5, 6, 8, 0, 0, 0, 0}, /* 0xB4 (10110100) */
    {1, 3, 5, 6, 8, 0, 0, 0}, /* 0xB5 (10110101) */
    {2, 3, 5, 6, 8, 0, 0, 0}, /* 0xB6 (10110110) */
    {1, 2, 3, 5, 6, 8, 0, 0}, /* 0xB7 (10110111) */
    {4, 5, 6, 8, 0, 0, 0, 0}, /* 0xB8 (10111000) */
    {1, 4, 5, 6, 8, 0, 0, 0}, /* 0xB9 (10111001) */
    {2, 4, 5, 6, 8, 0, 0, 0}, /* 0xBA (10111010) */
    {1, 2, 4, 5, 6, 8, 0, 0}, /* 0xBB (10111011) */
    {3, 4, 5, 6, 8, 0, 0, 0}, /* 0xBC (10111100) */
    {1, 3, 4, 5, 6, 8, 0, 0}, /* 0xBD (10111101) */
    {2, 3, 4, 5, 6, 8, 0, 0}, /* 0xBE (10111110) */
    {1, 2, 3, 4, 5, 6, 8, 0}, /* 0xBF (10111111) */
    {7, 8, 0, 0, 0, 0, 0, 0}, /* 0xC0 (11000000) */
    {1, 7, 8, 0, 0, 0, 0, 0}, /* 0xC1 (11000001) */
    {2, 7, 8, 0, 0, 0, 0, 0}, /* 0xC2 (11000010) */
    {1, 2, 7, 8, 0, 0, 0, 0}, /* 0xC3 (11000011) */
    {3, 7, 8, 0, 0, 0, 0, 0}, /* 0xC4 (11000100) */
    {1, 3, 7, 8, 0, 0, 0, 0}, /* 0xC5 (11000101) */
    {2, 3, 7, 8, 0, 0, 0, 0}, /* 0xC6 (11000110) */
    {1, 2, 3, 7, 8, 0, 0, 0}, /* 0xC7 (11000111) */
    {4, 7, 8, 0, 0, 0, 0, 0}, /* 0xC8 (11001000) */
    {1, 4, 7, 8, 0, 0, 0, 0}, /* 0xC9 (11001001) */
    {2, 4, 7, 8, 0, 0, 0, 0}, /* 0xCA (11001010) */
    {1, 2, 4, 7, 8, 0, 0, 0}, /* 0xCB (11001011) */
    {3, 4, 7, 8, 0, 0, 0, 0}, /* 0xCC (11001100) */
    {1, 3, 4, 7, 8, 0, 0, 0}, /* 0xCD (11001101) */
    {2, 3, 4, 7, 8, 0, 0, 0}, /* 0xCE (11001110) */
    {1, 2, 3, 4, 7, 8, 0, 0}, /* 0xCF (11001111) */
    {5, 7, 8, 0, 0, 0, 0, 0}, /* 0xD0 (11010000) */
    {1, 5, 7, 8, 0, 0, 0, 0}, /* 0xD1 (11010001) */
    {2, 5, 7, 8, 0, 0, 0, 0}, /* 0xD2 (11010010) */
    {1, 2, 5, 7, 8, 0, 0, 0}, /* 0xD3 (11010011) */
    {3, 5, 7, 8, 0, 0, 0, 0}, /* 0xD4 (11010100) */
    {1, 3, 5, 7, 8, 0, 0, 0}, /* 0xD5 (11010101) */
    {2, 3, 5, 7, 8, 0, 0, 0}, /* 0xD6 (11010110) */
    {1, 2, 3, 5, 7, 8, 0, 0}, /* 0xD7 (11010111) */
    {4, 5, 7, 8, 0, 0, 0, 0}, /* 0xD8 (11011000) */
    {1, 4, 5, 7, 8, 0, 0, 0}, /* 0xD9 (11011001) */
    {2, 4, 5, 7, 8, 0, 0, 0}, /* 0xDA (11011010) */
    {1, 2, 4, 5, 7, 8, 0, 0}, /* 0xDB (11011011) */
    {3, 4, 5, 7, 8, 0, 0, 0}, /* 0xDC (11011100) */
    {1, 3, 4, 5, 7, 8, 0, 0}, /* 0xDD (11011101) */
    {2, 3, 4, 5, 7, 8, 0, 0}, /* 0xDE (11011110) */
    {1, 2, 3, 4, 5, 7, 8, 0}, /* 0xDF (11011111) */
    {6, 7, 8, 0, 0, 0, 0, 0}, /* 0xE0 (11100000) */
    {1, 6, 7, 8, 0, 0, 0, 0}, /* 0xE1 (11100001) */
    {2, 6, 7, 8, 0, 0, 0, 0}, /* 0xE2 (11100010) */
    {1, 2, 6, 7, 8, 0, 0, 0}, /* 0xE3 (11100011) */
    {3, 6, 7, 8, 0, 0, 0, 0}, /* 0xE4 (11100100) */
    {1, 3, 6, 7, 8, 0, 0, 0}, /* 0xE5 (11100101) */
    {2, 3, 6, 7, 8, 0, 0, 0}, /* 0xE6 (11100110) */
    {1, 2, 3, 6, 7, 8, 0, 0}, /* 0xE7 (11100111) */
    {4, 6, 7, 8, 0, 0, 0, 0}, /* 0xE8 (11101000) */
    {1, 4, 6, 7, 8, 0, 0, 0}, /* 0xE9 (11101001) */
    {2, 4, 6, 7, 8, 0, 0, 0}, /* 0xEA (11101010) */
    {1, 2, 4, 6, 7, 8, 0, 0}, /* 0xEB (11101011) */
    {3, 4, 6, 7, 8, 0, 0, 0}, /* 0xEC (11101100) */
    {1, 3, 4, 6, 7, 8, 0, 0}, /* 0xED (11101101) */
    {2, 3, 4, 6, 7, 8, 0, 0}, /* 0xEE (11101110) */
    {1, 2, 3, 4, 6, 7, 8, 0}, /* 0xEF (11101111) */
    {5, 6, 7, 8, 0, 0, 0, 0}, /* 0xF0 (11110000) */
    {1, 5, 6, 7, 8, 0, 0, 0}, /* 0xF1 (11110001) */
    {2, 5, 6, 7, 8, 0, 0, 0}, /* 0xF2 (11110010) */
    {1, 2, 5, 6, 7, 8, 0, 0}, /* 0xF3 (11110011) */
    {3, 5, 6, 7, 8, 0, 0, 0}, /* 0xF4 (11110100) */
    {1, 3, 5, 6, 7, 8, 0, 0}, /* 0xF5 (11110101) */
    {2, 3, 5, 6, 7, 8, 0, 0}, /* 0xF6 (11110110) */
    {1, 2, 3, 5, 6, 7, 8, 0}, /* 0xF7 (11110111) */
    {4, 5, 6, 7, 8, 0, 0, 0}, /* 0xF8 (11111000) */
    {1, 4, 5, 6, 7, 8, 0, 0}, /* 0xF9 (11111001) */
    {2, 4, 5, 6, 7, 8, 0, 0}, /* 0xFA (11111010) */
    {1, 2, 4, 5, 6, 7, 8, 0}, /* 0xFB (11111011) */
    {3, 4, 5, 6, 7, 8, 0, 0}, /* 0xFC (11111100) */
    {1, 3, 4, 5, 6, 7, 8, 0}, /* 0xFD (11111101) */
    {2, 3, 4, 5, 6, 7, 8, 0}, /* 0xFE (11111110) */
    {1, 2, 3, 4, 5, 6, 7, 8}  /* 0xFF (11111111) */
};

#endif

#if USEAVX

size_t bitset_extract_setbits_avx2(uint64_t *array, size_t length,
                                   uint32_t *out, size_t outcapacity,
                                   uint32_t base) {
    uint32_t *initout = out;
    __m256i baseVec = _mm256_set1_epi32(base - 1);
    __m256i incVec = _mm256_set1_epi32(64);
    __m256i add8 = _mm256_set1_epi32(8);
    uint32_t *safeout = out + outcapacity;
    size_t i = 0;
    for (; (i < length) && (out + 64 <= safeout); ++i) {
        uint64_t w = array[i];
        if (w == 0) {
            baseVec = _mm256_add_epi32(baseVec, incVec);
        } else {
            for (int k = 0; k < 4; ++k) {
                uint8_t byteA = (uint8_t)w;
                uint8_t byteB = (uint8_t)(w >> 8);
                w >>= 16;
                __m256i vecA =
                    _mm256_load_si256((const __m256i *)vecDecodeTable[byteA]);
                __m256i vecB =
                    _mm256_load_si256((const __m256i *)vecDecodeTable[byteB]);
                uint8_t advanceA = lengthTable[byteA];
                uint8_t advanceB = lengthTable[byteB];
                vecA = _mm256_add_epi32(baseVec, vecA);
                baseVec = _mm256_add_epi32(baseVec, add8);
                vecB = _mm256_add_epi32(baseVec, vecB);
                baseVec = _mm256_add_epi32(baseVec, add8);
                _mm256_storeu_si256((__m256i *)out, vecA);
                out += advanceA;
                _mm256_storeu_si256((__m256i *)out, vecB);
                out += advanceB;
            }
        }
    }
    base += i * 64;
    for (; (i < length) && (out < safeout); ++i) {
        uint64_t w = array[i];
        while ((w != 0) && (out < safeout)) {
            uint64_t t = w & -w;
            int r = __builtin_ctzll(w);
            *out = r + base;
            out++;
            w ^= t;
        }
        base += 64;
    }
    return out - initout;
}
#endif // USEAVX

size_t bitset_extract_setbits(uint64_t *bitset, size_t length, uint32_t *out,
                              uint32_t base) {
    int outpos = 0;
    for (size_t i = 0; i < length; ++i) {
        uint64_t w = bitset[i];
        while (w != 0) {
            uint64_t t = w & -w;
            int r = __builtin_ctzll(w);
            out[outpos++] = r + base;
            w ^= t;
        }
        base += 64;
    }
    return outpos;
}

size_t bitset_extract_intersection_setbits_uint16(const uint64_t *bitset1,
                                                  const uint64_t *bitset2,
                                                  size_t length, uint16_t *out,
                                                  uint16_t base) {
    int outpos = 0;
    for (size_t i = 0; i < length; ++i) {
        uint64_t w = bitset1[i] & bitset2[i];
        while (w != 0) {
            uint64_t t = w & -w;
            int r = __builtin_ctzll(w);
            out[outpos++] = r + base;
            w ^= t;
        }
        base += 64;
    }
    return outpos;
}

#ifdef IS_X64
/*
 * Given a bitset containing "length" 64-bit words, write out the position
 * of all the set bits to "out" as 16-bit integers, values start at "base" (can
 *be set to zero).
 *
 * The "out" pointer should be sufficient to store the actual number of bits
 *set.
 *
 * Returns how many values were actually decoded.
 *
 * This function uses SSE decoding.
 */
size_t bitset_extract_setbits_sse_uint16(const uint64_t *bitset, size_t length,
                                         uint16_t *out, size_t outcapacity,
                                         uint16_t base) {
    uint16_t *initout = out;
    __m128i baseVec = _mm_set1_epi16(base - 1);
    __m128i incVec = _mm_set1_epi16(64);
    __m128i add8 = _mm_set1_epi16(8);
    uint16_t *safeout = out + outcapacity;
    const int numberofbytes = 2;  // process two bytes at a time
    size_t i = 0;
    for (; (i < length) && (out + numberofbytes * 8 <= safeout); ++i) {
        uint64_t w = bitset[i];
        if (w == 0) {
            baseVec = _mm_add_epi16(baseVec, incVec);
        } else {
            for (int k = 0; k < 4; ++k) {
                uint8_t byteA = (uint8_t)w;
                uint8_t byteB = (uint8_t)(w >> 8);
                w >>= 16;
                __m128i vecA = _mm_load_si128(
                    (const __m128i *)vecDecodeTable_uint16[byteA]);
                __m128i vecB = _mm_load_si128(
                    (const __m128i *)vecDecodeTable_uint16[byteB]);
                uint8_t advanceA = lengthTable[byteA];
                uint8_t advanceB = lengthTable[byteB];
                vecA = _mm_add_epi16(baseVec, vecA);
                baseVec = _mm_add_epi16(baseVec, add8);
                vecB = _mm_add_epi16(baseVec, vecB);
                baseVec = _mm_add_epi16(baseVec, add8);
                _mm_storeu_si128((__m128i *)out, vecA);
                out += advanceA;
                _mm_storeu_si128((__m128i *)out, vecB);
                out += advanceB;
            }
        }
    }
    base += i * 64;
    for (; (i < length) && (out < safeout); ++i) {
        uint64_t w = bitset[i];
        while ((w != 0) && (out < safeout)) {
            uint64_t t = w & -w;
            int r = __builtin_ctzll(w);
            *out = r + base;
            out++;
            w ^= t;
        }
        base += 64;
    }
    return out - initout;
}
#endif


/*
 * Given a bitset containing "length" 64-bit words, write out the position
 * of all the set bits to "out", values start at "base" (can be set to zero).
 *
 * The "out" pointer should be sufficient to store the actual number of bits
 *set.
 *
 * Returns how many values were actually decoded.
 */
size_t bitset_extract_setbits_uint16(const uint64_t *bitset, size_t length,
                                     uint16_t *out, uint16_t base) {
    int outpos = 0;
    for (size_t i = 0; i < length; ++i) {
        uint64_t w = bitset[i];
        while (w != 0) {
            uint64_t t = w & -w;
            int r = __builtin_ctzll(w);
            out[outpos++] = r + base;
            w ^= t;
        }
        base += 64;
    }
    return outpos;
}

#ifdef ASMBITMANIPOPTIMIZATION

uint64_t bitset_set_list_withcard(void *bitset, uint64_t card,
                                  const uint16_t *list, uint64_t length) {
    uint64_t offset, load, pos;
    uint64_t shift = 6;
    const uint16_t *end = list + length;
    if (!length) return card;
    // bts is not available as an intrinsic in GCC
    __asm volatile(
        "1:\n"
        "movzwq (%[list]), %[pos]\n"
        "shrx %[shift], %[pos], %[offset]\n"
        "mov (%[bitset],%[offset],8), %[load]\n"
        "bts %[pos], %[load]\n"
        "mov %[load], (%[bitset],%[offset],8)\n"
        "sbb $-1, %[card]\n"
        "add $2, %[list]\n"
        "cmp %[list], %[end]\n"
        "jnz 1b"
        : [card] "+&r"(card), [list] "+&r"(list), [load] "=&r"(load),
          [pos] "=&r"(pos), [offset] "=&r"(offset)
        : [end] "r"(end), [bitset] "r"(bitset), [shift] "r"(shift));
    return card;
}

void bitset_set_list(void *bitset, const uint16_t *list, uint64_t length) {
    uint64_t offset, load, pos;
    uint64_t shift = 6;
    const uint16_t *end = list + length;
    if (!length) return;
    // bts is not available as an intrinsic in GCC
    __asm volatile(
        "1:\n"
        "movzwq (%[list]), %[pos]\n"
        "shrx %[shift], %[pos], %[offset]\n"
        "mov (%[bitset],%[offset],8), %[load]\n"
        "bts %[pos], %[load]\n"
        "mov %[load], (%[bitset],%[offset],8)\n"
        "add $2, %[list]\n"
        "cmp %[list], %[end]\n"
        "jnz 1b"
        : [list] "+&r"(list), [load] "=&r"(load), [pos] "=&r"(pos),
          [offset] "=&r"(offset)
        : [end] "r"(end), [bitset] "r"(bitset), [shift] "r"(shift));
}

uint64_t bitset_clear_list(void *bitset, uint64_t card, const uint16_t *list,
                           uint64_t length) {
    uint64_t offset, load, pos;
    uint64_t shift = 6;
    const uint16_t *end = list + length;
    if (!length) return card;
    // btr is not available as an intrinsic in GCC
    __asm volatile(
        "1:\n"
        "movzwq (%[list]), %[pos]\n"
        "shrx %[shift], %[pos], %[offset]\n"
        "mov (%[bitset],%[offset],8), %[load]\n"
        "btr %[pos], %[load]\n"
        "mov %[load], (%[bitset],%[offset],8)\n"
        "sbb $0, %[card]\n"
        "add $2, %[list]\n"
        "cmp %[list], %[end]\n"
        "jnz 1b"
        : [card] "+&r"(card), [list] "+&r"(list), [load] "=&r"(load),
          [pos] "=&r"(pos), [offset] "=&r"(offset)
        : [end] "r"(end), [bitset] "r"(bitset), [shift] "r"(shift)
        :
        /* clobbers */ "memory");
    return card;
}

#else

uint64_t bitset_clear_list(void *bitset, uint64_t card, const uint16_t *list,
                           uint64_t length) {
    uint64_t offset, load, newload, pos, index;
    const uint16_t *end = list + length;
    while (list != end) {
        pos = *(const uint16_t *)list;
        offset = pos >> 6;
        index = pos % 64;
        load = ((uint64_t *)bitset)[offset];
        newload = load & ~(UINT64_C(1) << index);
        card -= (load ^ newload) >> index;
        ((uint64_t *)bitset)[offset] = newload;
        list++;
    }
    return card;
}

uint64_t bitset_set_list_withcard(void *bitset, uint64_t card,
                                  const uint16_t *list, uint64_t length) {
    uint64_t offset, load, newload, pos, index;
    const uint16_t *end = list + length;
    while (list != end) {
        pos = *(const uint16_t *)list;
        offset = pos >> 6;
        index = pos % 64;
        load = ((uint64_t *)bitset)[offset];
        newload = load | (UINT64_C(1) << index);
        card += (load ^ newload) >> index;
        ((uint64_t *)bitset)[offset] = newload;
        list++;
    }
    return card;
}

void bitset_set_list(void *bitset, const uint16_t *list, uint64_t length) {
    uint64_t offset, load, newload, pos, index;
    const uint16_t *end = list + length;
    while (list != end) {
        pos = *(const uint16_t *)list;
        offset = pos >> 6;
        index = pos % 64;
        load = ((uint64_t *)bitset)[offset];
        newload = load | (UINT64_C(1) << index);
        ((uint64_t *)bitset)[offset] = newload;
        list++;
    }
}

#endif

/*
 * Set all bits in indexes [begin,end) to true.
 */
void bitset_set_range(uint64_t *bitmap, uint32_t start, uint32_t end) {
    if (start == end) return;
    /*for(uint32_t i = start; i < end ; ++i) {
        bitmap[i/64] |= (UINT64_C(1) << (i%64));
    }*/
    uint32_t firstword = start / 64;
    uint32_t endword = (end - 1) / 64;
    if (firstword == endword) {
        bitmap[firstword] |= ((~UINT64_C(0)) << (start % 64)) &
                             ((~UINT64_C(0)) >> ((-end) % 64));
        return;
    }
    bitmap[firstword] |= (~UINT64_C(0)) << (start % 64);
    for (uint32_t i = firstword + 1; i < endword; i++) bitmap[i] = ~UINT64_C(0);
    bitmap[endword] |= (~UINT64_C(0)) >> ((-end) % 64);
}

/*
 * Flip all the bits in indexes [begin,end).
 */

// TODO: AVX version
void bitset_flip_range(uint64_t *bitmap, uint32_t start, uint32_t end) {
    if (start == end) return;
    uint32_t firstword = start / 64;
    uint32_t endword = (end - 1) / 64;
    bitmap[firstword] ^= ~((~UINT64_C(0)) << (start % 64));
    for (uint32_t i = firstword; i < endword; i++) bitmap[i] = ~bitmap[i];
    bitmap[endword] ^= ((~UINT64_C(0)) >> ((-end) % 64));
}

/*
 * Set all bits in indexes [begin,end) to false.
 */
void bitset_reset_range(uint64_t *bitmap, uint32_t start, uint32_t end) {
    if (start == end) return;
    uint32_t firstword = start / 64;
    uint32_t endword = (end - 1) / 64;
    if (firstword == endword) {
        bitmap[firstword] &= ~(((~UINT64_C(0)) << (start % 64)) &
                               ((~UINT64_C(0)) >> ((-end) % 64)));
        return;
    }
    bitmap[firstword] &= ~((~UINT64_C(0)) << (start % 64));
    for (uint32_t i = firstword + 1; i < endword; i++) bitmap[i] = UINT64_C(0);
    bitmap[endword] &= ~((~UINT64_C(0)) >> ((-end) % 64));
}
