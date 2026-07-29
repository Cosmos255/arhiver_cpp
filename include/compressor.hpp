#pragma once
#include "../include/utils.hpp"
#include "../include/lz77.hpp"
#include "../include/arhv.hpp"
#include <cstdio>
#include <memory>
#include <cstdint>



struct FLC {
    int freq=0;
    int len=0;
    uint64_t code =0;
};

struct lenCodes{
    int symbol = 0;
    int extra = 0;
};

const int MAX_BITS = 15;

constexpr int maxCodeValue = 286;
constexpr int maxDistValue = 30;
constexpr int OUTBUFFER_SIZE = 1<<20; //1MB


void buildTree();
void createLen(std::vector<bits> &val_len, FLC *arr, int i, uint16_t len);
void createCodes(FLC *arr);
void createCCLFreq(const int prev, int &prev_count);
void processDictionaryRun();
uint64_t revCodes(uint64_t code, int len);
void writeBitcode(uint64_t code, int len);
void writeExtra(int key, const bool len);
void flush(bool forced= false);
void outputCCLCode(int key, int extra);
void outputDictionary();
void writeBits(uint64_t code, int len);
void writeStored();
void HeaderInit(bool lastblock, c_type mode);
static void deflateInit();