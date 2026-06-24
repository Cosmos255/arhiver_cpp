#include "../include/utils.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <assert.h>
#include <vector>
#include <unordered_map>
#include <exception>
#include <algorithm>

std::fstream in;
std::ofstream out;

constexpr int INBUFFER_SIZE = 16000; //16KB
constexpr int OUTBUFFER_SIZE = 1<<20; //1MB
constexpr int WINDOW_SIZE = 33000;


struct canonical_struct{
    int len = 0; //len/code
    int symbol = 0; //the index in the array 
};



struct Header{
    bool final = 0;
    bool compressed = 0;
    bool dynamicHuffman = 0; // false
    bool reserved = 0;
    bool ALL_LITERALS = false;
    int HLIT=0;
    int HDIST=0;
    int HCLEN=0;

}header_settings;


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


std::vector<uint8_t> window(WINDOW_SIZE);
std::vector<uint8_t> inbuffer(INBUFFER_SIZE);
std::vector<uint8_t> outbuffer(OUTBUFFER_SIZE);


uint64_t window_indx;
uint64_t window_notav;

int outbuff_indx = 0;

uint8_t *pBitpos = inbuffer.data();
uint8_t *pBitpos_end = pBitpos + INBUFFER_SIZE;

const int INPUT_SIZE = 10000;

uint64_t bitbuf = 0;
int bitpos=0;

void test();

std::unordered_map<int, int> mp_literals;
std::unordered_map<int, int> mp_distance;

//rename it do decompress and pass intput file name and output file name/locations

bool main(int argc, char* argv[]){
    try{
    if(argc < 3) throw std::runtime_error("Not enough arguments");
    
    std::string in_filepath = argv[1];
    std::string out_filepath = argv[2];
    
    if(in_filepath.empty() || out_filepath.empty()) throw std::runtime_error("Filepaths are empty");

    in.open(in_filepath, std::ios::in | std::ios::binary);
    out.open(out_filepath, std::ios::out | std::ios::trunc | std::ios::binary);

    if(!in.is_open()) throw std::runtime_error("Couldnt open the input file");
    in.read(reinterpret_cast<char *>(inbuffer.data()), INBUFFER_SIZE); //idk windows size

    pBitpos_end = inbuffer.data() + in.gcount();

    if(!out.is_open()) throw std::runtime_error("Couldnt open the output file");

    std::fill(window.begin(), window.end(), 0);
    window_indx = 0;
    window_notav = WINDOW_SIZE;

    /*
    ##############################################
    consumebits(1)
    Function for filling bitbuff
    !!Only use when bitbuff is empty or for inital
    fill it other cases dont pass any argument

    ##############################################
    */

    consumebits(1);



    while(true){
        header_settings = {}; //initialize struct to defaults

        mp_literals.clear();
        mp_distance.clear();

    

        readHeadderType();


        if(!header_settings.compressed){
            getbits(5); //padding bits;
            int len = getbits(8) | (getbits(8)<<8);
            int nlen = getbits(8) | (getbits(8)<<8);

            if(!nlen == (~len) & 0xFFFF) throw std::runtime_error("Stream invalid nlen not compliment of len");

            while(len--){
                outBuffWrite(getbits(8));
            }
        }else{
    
        if(header_settings.dynamicHuffman) readBlockFormat();
        else initializeFixedMap();

        std::cout<<"HLIT: "<<header_settings.HLIT;
        std::cout<<"HDIST: "<<header_settings.HDIST;
        std::cout<<"HCLEN: "<<header_settings.HCLEN;
    
        while(true){
            int symbol = parseLiteral();
            if(symbol < 0) throw std::runtime_error("Unexpected error: Symbol is negative");

            if(symbol == 256){
                outBuffWrite(0, 1);
                break;
            }
        
            if(symbol >=257){
                int len = parseLiteral(1, symbol);
                int dist = parseDist();

                findMatch(len, dist); //calls a function that handles buffinsertions
                continue;
            }else if (symbol < 257){
                outBuffWrite(symbol);
                continue;
            }
            throw std::runtime_error("Unexpected finish");
        }
        }

        if(header_settings.final) break;
    }
    
    outBuffWrite(0, 1);

    out.close();
    in.close();

    cleanup();



    }
    catch(const std::exception& e){
        outBuffWrite(0, 1); //idk if i should dump when error
        out.close();
        in.close();
        cleanup();
        window.clear();
        outbuffer.clear();
        inbuffer.clear();
        std::cerr<<e.what()<<"\n";
        return 1; //error
    }

    std::cout<<"\nYes finnaly";

    return 0;
}

void fillInbuff(){
    if(pBitpos > pBitpos_end) throw std::runtime_error("Error: Inbuffer pointer past buffer end");

    auto len = pBitpos_end-pBitpos;//how many symbols are still valid

    if(len < 0) throw std::runtime_error("Error: len is negative when filling the buffer");


    if(len) std::rotate(inbuffer.begin(), inbuffer.begin() + std::distance(inbuffer.data(), pBitpos), inbuffer.end());//should rotate

    in.read(reinterpret_cast<char *>(inbuffer.data()+len), INBUFFER_SIZE-len);

    if(in.gcount() > INBUFFER_SIZE-len) throw std::runtime_error("Error: in.gcount is bigger than expected");

    pBitpos_end = inbuffer.data()+in.gcount()+len;
    pBitpos = inbuffer.data();

}

void consumebits(const bool empty){
    int rm;

    if(empty){
        std::cerr<<"\nWarning! usage of emptybuffer argument\n";
        bitbuf = 0;
        bitpos = 0;
        rm=64;
    }else{
        rm = bitpos - (bitpos%8); //bits to remove
        if(rm <=0) throw std::runtime_error("Error: consumebits remove is 0 or lower");
        
        if(rm == 64) bitbuf = 0;
        else bitbuf>>=rm;

        if(bitpos < rm) throw std::runtime_error("Error: remove is bigger than bitpos");

        bitpos-=rm;
    }

    if(pBitpos > pBitpos_end) throw std::runtime_error("Error: Buffer pointer past buffer end");

    if(rm/8 > pBitpos_end-pBitpos) fillInbuff();


    while(rm > 0 && pBitpos != pBitpos_end){
        if(rm < 8 || rm > 64) throw std::runtime_error("Error: remove is desynced");
        bitbuf|=(uint64_t)(*pBitpos) << (64-rm);
        rm-=8;
        pBitpos++;
    }

}

uint32_t getbits(int len){
    if(len == 0) return 0;
    if(len < 0 || len > 15) throw std::runtime_error("Error: getbits len is negative or bigger than 15");//max bits 15
    uint32_t mask = (1u << len)-1;
    if(bitpos + len >= 64) consumebits(); //not enough bits;
    bitpos+=len;
    return (bitbuf >> (bitpos-len)) & mask;

}


void readHeadderType(){
    header_settings.final = getbits(1); // ((bitbuf>>bitpos) & 0b1);
    switch (getbits(2)){ //(bitbuf>>bitpos)&0b11
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

}

void initializeFixedMap(){
    /*
    Only use with fixed type 01
    */
    int e=0;
    for(int i= 48; i<=191; i++) mp_literals.insert({(1<<8)| i, e++});
    for(int i= 400; i<=511; i++) mp_literals.insert({(1<<9) | i, e++});
    for(int i=0; i<=23; i++) mp_literals.insert({(1<<7)| i, e++});
    for(int i=192; i<=199; i++) mp_literals.insert({(1<<8) | i, e++});

    for(int i=0; i<=31; i++){
        //5bits long
        mp_distance.insert({(1 << 5)|i, i});
    }

}

void readBlockFormat(){

    int hlit = getbits(5); //(bitbuf>>bitpos) & 0x1F;
    //bitpos+=5;
    int hdist = getbits(5); //(bitbuf>>bitpos) & 0x1F;
    //bitpos+=5;
    int hclen = getbits(4); //(bitbuf>>bitpos) & 0xF;
    //bitpos+=4;
    header_settings.HLIT = hlit+257; 
    header_settings.HDIST = hdist+1;    
    header_settings.HCLEN  = hclen+4;


    std::unordered_map<int, int> mp; //code arr_index

    if(header_settings.HCLEN < 4 || header_settings.HCLEN >19) throw std::runtime_error("HCLEN is invalid");

    std::vector<canonical_struct> code_len(header_settings.HCLEN);

    for(int i=0; i<code_len.size(); i++){
        code_len[i].len = getbits(3);
        code_len[i].symbol = CCL_order[i];
    }
    
    //sorting
    merge::sort(code_len,[](canonical_struct &a, canonical_struct &b){
        if(a.len == b.len) return a.symbol < b.symbol;
        return a.len < b.len;
    });


    canonicalHuffman(code_len, mp);

        

    //this should be all for HCLEN
    //use the HCLEN to do the HLIT and HDIST
    //HLIT

    std::vector<int> len_vec = constructLenArray(mp);//len for both HDIST and HLIT


    std::vector<canonical_struct> literal_len(header_settings.HLIT);
    std::vector<canonical_struct> distance_len(header_settings.HDIST);

    for(int i=0; i<header_settings.HLIT; i++){
        literal_len[i].len = len_vec[i];
        literal_len[i].symbol = i;
    }

    merge::sort(literal_len, [](canonical_struct &a, canonical_struct &b){
        if(a.len == b.len) return a.symbol < b.symbol;
        return a.len < b.len;
    });

    canonicalHuffman(literal_len, mp_literals);

    //HDIST
    
    for(int i=header_settings.HLIT; i<len_vec.size(); i++){
        int j = i - header_settings.HLIT;
        distance_len[j].len = len_vec[i];
        distance_len[j].symbol = j;
    }

    if(distance_len.size() == 1 && distance_len[0].len == 0){
        header_settings.ALL_LITERALS = true;
    }else{



        merge::sort(distance_len, [](canonical_struct &a, canonical_struct &b){
            if(a.len == b.len) return a.symbol < b.symbol;
            return a.len < b.len;
        });
        canonicalHuffman(distance_len, mp_distance);
    }

}

std::vector<int> constructLenArray(std::unordered_map<int, int> &mp){
    
    std::vector<int> len_vec(header_settings.HLIT + header_settings.HDIST);

    auto it = len_vec.begin();
    while(it < len_vec.end()){
        int code = 1;
        int len = 0;
        std::unordered_map<int,int>::iterator mit;

        do{
            if(len > 15) throw std::runtime_error("constructLenArray: code too long");
            code  = (code << 1) | getbits(1);
            len++;
            mit = mp.find(code);

        }while(mit == mp.end());


        if(mit->second < 0 || mit->second > 18) throw std::runtime_error("Code index invalid"); //prob useless but doesnt hurt

        int fill_symbol = 0;
        int fill_size = 0;
        int extra_len;

        switch(mit->second){
            case 18:
                extra_len = getbits(7);
                fill_size = 11+extra_len;
                if(11+extra_len > len_vec.end()-it) throw std::runtime_error("Brooken"); //idk :P
                fill_symbol = 0;
                break;
            case 17:    
                extra_len = getbits(3);
                fill_size = 3+extra_len;
                fill_symbol = 0;
                break;
            case 16:
                if(it==len_vec.begin()) throw std::runtime_error("HLIT cant start with len code 16");
                extra_len = getbits(2);
                fill_size = 3+extra_len;
                fill_symbol = *(it-1);
                break;
            default:
                fill_symbol = mit->second;
                fill_size = 1;
        }
        if(it+fill_size > len_vec.end()) throw std::runtime_error("Fill size for len codes is larger than HDIST+HLIT");
        std::fill(it, it+fill_size, fill_symbol);
        it+=fill_size;
    }

    return len_vec;
}

void canonicalHuffman(std::vector<canonical_struct> &arr, std::unordered_map<int, int> &mp){
    if(!arr.size()) throw std::runtime_error("Error: Canonical Huffman arr is empty");

    int len=0;
    int code=0;

    for(auto &obj : arr){
        if(obj.len == 0) continue;
        if(obj.len > 15 || obj.len < 0) throw std::runtime_error("Error: Canonical Huffman len is bigger than 15 or negative");
        if(obj.len > len) code <<= (obj.len-len);
        len=obj.len;
        mp.insert({(1<<len)| code , obj.symbol});
        code++;
    }
}

int parseCode(std::unordered_map<int, int> &mp){
    int code = 1;
    int len = 0;
    while(len <= 15){
        code = (code << 1) | getbits(1);
        len++;
        auto it = mp.find(code);
        if(it != mp.end()) return it->second;
    }

    throw std::runtime_error("parseLiteral: Code len bigger than 15, couldnt find the code");
}

int parseLiteral(bool parseLen, int symbol){
    if(symbol > 285) throw std::runtime_error("Error: Literal symbol bigger than 285");
    if(parseLen){
        if(symbol < 257) throw std::runtime_error("Error: Length symbol lower than 257");
        
        int len = lengthTable[symbol-257].baselength;
        int extra_bits = lengthTable[symbol-257].extraBits;

        int extra_len = getbits(extra_bits);

        return len+extra_len;
    }
    return parseCode(mp_literals);
}

int parseDist(){
    auto symbol = parseCode(mp_distance);
    if(symbol >= 30) throw std::runtime_error("Error: Distance index is bigger or equal to 30");

    int dist = distTable[symbol].basedist;
    int extra_bits = distTable[symbol].extraBits;

    int extra_dist = getbits(extra_bits);
    
    return dist+extra_dist;
}

void findMatch(int len, int dist){
    if(dist > WINDOW_SIZE || dist > window_indx) throw std::runtime_error("Dist is bigger than 36KB or windwos is filled");

    uint64_t lookup = window_indx-dist;

    while(len){
        outBuffWrite(window[lookup%WINDOW_SIZE]);
        lookup++;
        len--;
    }
}

void outBuffWrite(uint8_t ch, bool f_out){
    if(outbuff_indx >= outbuffer.size() || f_out){
        int attempts = 0;
        while(attempts < 2){
            try{
                out.write(reinterpret_cast<char*>(outbuffer.data()), outbuff_indx);

                if(!out || out.fail()){
                    throw std::runtime_error("write failed");
                }
                break;

            }catch(const std::exception&){
                out.clear();
                attempts++;
                if(attempts == 2) {
                    cleanup();
                    throw std::runtime_error("Failed to write execution stopped");
                }
            }
        }
        outbuff_indx=0;
    }

    if(!f_out){
        outbuffer[outbuff_indx] = ch;//add ch to outbuff
        window[window_indx%WINDOW_SIZE] = ch;
        window_indx++;
        outbuff_indx++;
    }
}

void cleanup(){
    mp_literals.clear();
    mp_distance.clear();
}

