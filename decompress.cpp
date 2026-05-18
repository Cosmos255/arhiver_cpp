#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <assert.h>

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

void test();

int main(){
    if(!in.is_open()) throw std::runtime_error("Couldnt open the file");
    in.read(reinterpret_cast<char *>(inbuffer), sizeof(inbuffer)); //idk windows size
    //readHeadderType();

    std::cout<<"HLIT: "<<header_settings.HLIT;
    std::cout<<"HDIST: "<<header_settings.HDIST;
    std::cout<<"HCLEN: "<<header_settings.HCLEN;

   test();

    //fillBitBuff();


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
/*
void readAlfa();

//Note the shift is moded so if buffer <<=64 then it does nothing
//

void fillBitBuff(){
    int bits = bitpos/8;
    bits*=8;
    bitbuf>>=bits;
    bitpos-=bits;
    while (bits > 0){
        bits-=8;
        if(56-bits)
        bitbuf|= (uint64_t)(*pBitpos++) << (56 - bits);
    }
}

void readCodes(){

}

void readDist(){

}

void readExtra(){

}

void buildCanonical(){

}

uint8_t extractBitBuff(int len){*
*/

void fillInbuff(){
    //maybe do a windows like in lz77a

    int len = pBitpos_end-pBitpos;
    assert (len >=0);
    if(len) memmove(inbuffer, pBitpos, len);

    in.read(reinterpret_cast<char *>(inbuffer+len), INBUFFER_SIZE-len);

    assert(in.gcount() <= INBUFFER_SIZE-len);
    if(in.gcount() != INBUFFER_SIZE-len) pBitpos_end = inbuffer+in.gcount()+len; 
    pBitpos = inbuffer;

}

void moveBitBuffwindows(){
    //add function to insert the data into the window
    int rm = bitpos - (bitpos%8);
    assert(rm >= 0);
    
    if(rm == 64) bitbuf=0; //mobing by buff size will do nothing
    else bitbuf>>=rm;
    bitpos-=rm;
    
    if( rm/8 > abs(pBitpos_end-pBitpos)) fillInbuff();

    while(rm > 0 && pBitpos != pBitpos_end){
        bitbuf|=(uint64_t)(*pBitpos++) << (64 - rm);
        rm-=8;
    }

}

uint32_t getbits(int len){
    assert(len <= 15); //max bits 15
    assert(len >= 0);
    uint32_t mask = (1u << len) -1;
    if(bitpos + len > 64) moveBitBuffwindows(); //not enough bits;
    bitpos+=len;
    return (bitbuf >> (bitpos-len)) & mask;
}
//add smth to 


void vHEXdump(const uint8_t *arr, int len){
    //uint8_t *pAar = (uint8_t*)arr;

    for(int i=0; i<len; i++){
        printf("%02X ", arr[i]);
    }

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

void test(){
    std::string test = "Hello this is a test i the test code is 5555 and the user name is lain iwakura";
    memcpy(inbuffer, test.data(), test.size());
    pBitpos_end = inbuffer + test.size();
    pBitpos = inbuffer;
    bitpos = 64;
    moveBitBuffwindows();

    uint8_t byte;

    const int x = test.size();

    uint8_t *tmp = new uint8_t[test.size()*2];
    uint8_t *pos = &tmp[0];

    try{
        while(pBitpos != pBitpos_end || bitbuf){ //maybe use bitpos instead of bitbuff
            *pos = getbits(4);
            pos++;
        }
    }catch(...){
        std::cout<<"\n\n\n";
        vHEXdump(tmp, test.size());
    }
    std::cout<<"\n\n\n";
    vHEXdump(tmp, test.size()*2);
}
