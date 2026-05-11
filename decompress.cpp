#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>

std::fstream in("out.bin", std::ios::in | std::ios::binary);

const int INBUFFER_SIZE = 16*1024;


struct Header{
    bool final = 0;
    bool compressed = 0;
    bool dynamicHuffman = 0; // false
    bool reserved = 0;
    int HLIT=0;
    int HDIST=0;
    int HCLEN=0;
}header_settings;

void HEXdump(uint32_t arr);
void readHeadderType();




uint8_t window[32768];
uint8_t inbuffer[INBUFFER_SIZE]; // 

uint8_t *pBitpos = inbuffer;
uint8_t *pBitpos_end = inbuffer + INBUFFER_SIZE;

const int INPUT_SIZE = 10000;

uint64_t bitbuf;
int bitpos=0;

int main(){
    if(!in.is_open()) throw std::runtime_error("Couldnt open the file");
    in.read(reinterpret_cast<char *>(inbuffer), sizeof(inbuffer)); //idk windows size
    readHeadderType();

    std::cout<<"HLIT: "<<header_settings.HLIT;
    std::cout<<"HDIST: "<<header_settings.HDIST;
    std::cout<<"HCLEN: "<<header_settings.HCLEN;

    fillBitBuff();

}

//IDEA 
//WINDOW is only for the already proccesed codes that need to be outputtted and are used for matching
//INBUFFER is where we store the data we read
//bitbuff is the working buffer its where we fill reuse and move i think


void readHeadderType(){
    bitbuf = *pBitpos;
    header_settings.final = ((bitbuf>>bitpos) & 0b1);
    bitpos+=1;
    switch ((bitbuf>>bitpos)&0b11){
    case 3:
        header_settings.reserved = 1;
        throw std::runtime_error("Code 11 isnt allowed");
        break;
    case 2:
        header_settings.compressed =1;
        header_settings.dynamicHuffman = 1;
        break;
    case 1:
        header_settings.compressed =1;
        header_settings.dynamicHuffman = 0;
        break;
    default:
        header_settings.compressed = 0;
        break;
    }
    bitpos+=2;
    int hlit;
    hlit = (bitbuf>>bitpos) & 0x1F;
    bitpos+=5;
    int hdist;
    hdist = (bitbuf>>bitpos) & 0x1F;
    bitpos+=5;
    int hclen;
    hclen = (bitbuf>>bitpos) & 0xF;
    bitpos+=4;

    header_settings.HLIT = hlit+257;
    header_settings.HDIST = hdist+1;    
    header_settings.HCLEN  = hclen+4;

    HEXdump(bitbuf);

}

void readAlfa();

void fillBitBuff(){
    int bits = bitpos/8;
    bits*=8;
    bitbuf>>=bits;
    bitpos-=bits;
    while (bits > 0){
        bits-=8;
        bitbuf|= (uint64_t)(*pBitpos++) << (56 - bits);
    }
}

void moveBitBuffwindows();

void readCodes(){

}

void readDist(){

}

void readExtra(){

}

void buildCanonical(){

}

uint8_t extractBitBuff(int len){
    if(bitpos+len > 32) moveBitBuffwindows();
    bitpos+=len;
}

void moveBitBuffwindows();

uint32_t getbits(int len){
    uint32_t mask = (1u << len) -1;
    if(bitpos + len > 32) moveBitBuffwindows();
    return (bitbuf >> bitpos) & mask;
}

void vHEXdump(void *arr, int len){
    uint8_t *pAar = (uint8_t*)arr;
}

void HEXdump(uint32_t arr){
    uint8_t tmp[4];
    for(int i=0; i<4; i++){
        tmp[i] = (arr & 0xFF);
        arr>>=8;
    }

    for(int i=0; i<4; i++){
        printf("%02X ", tmp[i]);
    }


    
}