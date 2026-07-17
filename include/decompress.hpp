#pragma once
#include "../include/utils.hpp"
#include "../include/arhv.hpp"
#include <vector>
#include <cstdio>
#include <unordered_map>

constexpr int INBUFFER_SIZE = 16000; //16KB
constexpr int OUTBUFFER_SIZE = 1<<20; //1MB
constexpr int WINDOW_SIZE = 33000;

struct canonical_struct{
    int len = 0; //len/code
    int symbol = 0; //the index in the array 
};

void vHEXdump(const uint8_t *arr, int len);
void HEXdump(uint32_t arr);
void readHeadderType();
void initializeFixedMap();
void readBlockFormat();
void canonicalHuffman(std::vector<canonical_struct> &arr, std::unordered_map<int, int> &mp);
std::vector<int> constructLenArray(std::unordered_map<int, int> &mp);
int parseLiteral(bool parseLen=0, int symbol = 0);
int parseDist();
uint32_t getbits(int len=0);
void findMatch(int len, int dist);
void outBuffWrite(uint8_t ch = 0, bool f_out = 0);
void cleanup();
void consumebits(const bool empty=0);
static void initialize();