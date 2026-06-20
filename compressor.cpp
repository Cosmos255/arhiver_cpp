#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>
#include "utils.hpp"
#include "lz77.hpp"

/*
NOTE: using a smaller buffer might reduce how much memory is allocated 
as we can switch to uint32t instead of 64



*/
//freq Len code
struct FLC {
    int freq=0;
    int len=0;
    uint64_t code =0;
};


const int MAX_BITS = 15;

constexpr int maxCodeValue = 286;
constexpr int maxDistValue = 30;

FLC literal[maxCodeValue] = {};
FLC dist[maxDistValue] = {};
FLC CCL[19] = {};


std::vector<bn_heap::Node> tree;

int HLIT=257;
int HDIST=1;
int HCLEN=4;


enum CCL_mode {WRITE, COUNT};



void processDictionaryRun(CCL_mode mode, uint64_t&buffer, int &bitpos);
void outputDictionary(const int prev, int &prev_count, uint64_t &buffer, int &bitpos);
void outputCCLCode(int key, int extra, uint64_t &buffer, int &bitpos);
void buildTree();
void createLen(std::vector<bits> &val_len, FLC *arr,  uint16_t len, int i);
void createCodes(FLC *arr);
uint64_t revCodes(uint64_t code, int len);
void writeBitcode(uint64_t code, int len, uint64_t &buffer, int &bitpos);
void writeExtra(int key, uint64_t &buffer, int &bitpos,  const bool len);
void flush(uint64_t &buffer, int &bitpos, bool final=0);

void printHuffmanTable(const char* name, FLC* arr, int size);
void printSortedLengths(FLC* arr, int size);

std::ofstream out("out.bin", std::ios::binary);




int main(){
    if (!out) {
        std::cerr << "Failed to open file\n";
    }

    //tree.reserve(600); //should add smth to improve performance

    auto lzed = lz77_token("example.txt");
    //creating a frequency array from tokens


    for(token &tk : lzed){
        if(tk.type == match){
            if(lcode(tk.len) < 0) throw std::runtime_error("Lcode is negative");
            if(dcode(tk.dist) < 0) throw std::runtime_error("Dcode is negative");



            literal[257 + lcode(tk.len)].freq++;
            dist[dcode(tk.dist)].freq++;
        }
        else literal[(uint8_t)(tk.data)].freq++;

    }
    for(int i=0; i<maxDistValue; i++){
        if(dist[i].freq) break;
        if(i == maxDistValue-1) dist[0].freq = 1;
    }
    literal[256].freq++;  //for end of block


    //literal length codes
    
    for(int i=0; i<maxCodeValue; i++)
    if(literal[i].freq){
        bn_heap::insert({i, literal[i].freq});
    } 

    buildTree();
    createCodes(literal);
    tree.clear();
    bn_heap::clear();
    
    //dist codes
    for(int i=0; i<maxDistValue; i++)
    if(dist[i].freq){
        bn_heap::insert({i, dist[i].freq});
    } 

    buildTree();
    createCodes(dist);
    tree.clear();
    bn_heap::clear();

    uint64_t buffer = 0;
    int bitpos = 0; //max is 64

    processDictionaryRun(COUNT, buffer, bitpos);

    for(int i=0; i<19; i++)
        if(CCL[i].freq){
            bn_heap::insert({i, CCL[i].freq});
    }
    
    buildTree();
    createCodes(CCL);
    tree.clear();
    bn_heap::clear();


    printHuffmanTable("LITERAL/LEN CODES", literal, maxCodeValue);
    printHuffmanTable("DIST CODES", dist, maxDistValue);
    printHuffmanTable("CCL CODES", CCL, 19);

    printSortedLengths(literal, maxCodeValue);
    printSortedLengths(dist, maxDistValue);
    printSortedLengths(CCL, 19);




    //Creating the header and blocks and outputtign the data


    HLIT = maxCodeValue-1;
    while(HLIT >= 257 &&  literal[HLIT].len == 0){
        HLIT--;
    }

    HDIST = maxDistValue-1;
    while(HDIST >= 1 && dist[HDIST].len == 0){
        HDIST--;
    }

    HCLEN = 18;
    while (HCLEN >= 4 && CCL[CCL_order[HCLEN]].len == 0){
        HCLEN--;
    }


    
    HLIT= (HLIT+1)-257; 
    HDIST = (HDIST+1)-1; //indexing now this should really work as intented
    HCLEN=(HCLEN+1)-4; //x 3 bits

    if(HLIT < 0 || HDIST < 0 || HCLEN < 0) throw std::runtime_error("Unexpected negative values");
    

    //std::string buffer;

    buffer = 0;
    bitpos = 0; //max is 64


    bool compresed = true;
    bool dynamicHuffman = true;
    bool lastblock = true;

    if (lastblock) buffer |= 1;
    bitpos++;

    if(compresed){
        if(dynamicHuffman) buffer |= (0b10 << bitpos); 
        else buffer |= (0b01 << bitpos); 
    }else{
        buffer |= (0b11 << bitpos); //error
    }
    bitpos+=2;


    //add static cast uint64t to all codes that are written into the buffer
    //cuz its done in int space not uint


    //HLIT 5bits

    buffer |= (static_cast<uint64_t>(HLIT) << bitpos);
    bitpos+=5;


    //HDIST 5 bits

    buffer |= (static_cast<uint64_t>(HDIST) << bitpos);
    bitpos+=5;

    //HCLEN 4 bits

    buffer |= (static_cast<uint64_t>(HCLEN) << bitpos);
    bitpos+=4;


    std::cout<<"HLIT: "<<HLIT+257;
    std::cout<<"HDIST: "<<HDIST+1;
    std::cout<<"HCLEN: "<<HCLEN+4<<"\n";


    //CCL output



    for(int i=0; i<(HCLEN+4); i++){
        int key = CCL_order[i];
        if(CCL[key].len > 7) throw std::runtime_error("Too long aint ya");
        if(bitpos > 61) flush(buffer, bitpos);
        buffer|=(static_cast<uint64_t>(CCL[key].len & 0x7) << bitpos);
        bitpos+=3;
    }


    //Dictionay output with ccl
    processDictionaryRun(WRITE, buffer, bitpos);
    //outputting the compressed data

    for(token &t : lzed){
        if(t.type == match){
            writeBitcode(literal[257 + lcode(t.len)].code, literal[257 + lcode(t.len)].len, buffer, bitpos); //wait what about dcode and lcode ?? 
            writeExtra(t.len, buffer, bitpos, true);
            writeBitcode(dist[dcode(t.dist)].code, dist[dcode(t.dist)].len, buffer, bitpos);
            writeExtra(t.dist, buffer, bitpos, false);
        }
        else{
            writeBitcode(literal[t.data].code, literal[t.data].len, buffer, bitpos);
        }
    }

    writeBitcode(literal[256].code, literal[256].len, buffer, bitpos); //this should be it i think
    flush(buffer, bitpos, 1);

    out.close();
    return 0;

    std::cerr<<"THE HDIST HCLEN and HLIN arent computed correctly so it doesnt work";



    //now its decompresser time
}

void outputDictionary(const int prev, int &prev_count, uint64_t &buffer, int &bitpos){
     
    outputCCLCode(prev, 0, buffer, bitpos);
    prev_count--;

    int repeat_len;
    int extra;
    
    if(prev > 0) while(prev_count >= 3){
        repeat_len = std::min(prev_count, 6);
        extra = repeat_len-3;

        outputCCLCode(16, extra, buffer, bitpos);

        prev_count-=repeat_len;
    }
   

    if(prev == 0){
        while(prev_count >= 11){
            repeat_len = std::min(prev_count, 138);
            extra = repeat_len - 11;
            outputCCLCode(18, extra, buffer, bitpos);
            prev_count-=repeat_len;
        }
        while (prev_count >= 3){
            repeat_len = std::min(prev_count, 10);
            extra = repeat_len - 3;
            outputCCLCode(17, extra, buffer, bitpos);
            prev_count-=repeat_len;
        }
    }

    while(prev_count > 0){
        outputCCLCode(prev, 0, buffer, bitpos);
        prev_count--;
    }
}

void createCCLFreq(const int prev, int &prev_count){
    prev_count--;

    int repeat_len;
    int extra; 

    if(prev > 0) while(prev_count >= 3){
        repeat_len = std::min(prev_count, 6);
        CCL[16].freq += repeat_len;
        prev_count-=repeat_len;
    }
   

    if(prev == 0){
        while(prev_count >= 11){
            repeat_len = std::min(prev_count, 138);
            CCL[18].freq += repeat_len;
            prev_count-=repeat_len;
        }
        while (prev_count >= 3){
            repeat_len = std::min(prev_count, 10);
            CCL[17].freq += repeat_len;
            prev_count-=repeat_len;
        }
    }


    while(prev_count > 0){
        CCL[prev].freq += prev_count;
        prev_count = 0;
    }
}

void outputCCLCode(int key, int extra, uint64_t &buffer, int &bitpos){
    int extr_len;

    switch (key){
    case 16:
        extr_len = 2;
        break;
    case 17:
        extr_len = 3;
        break;
    case 18:
        extr_len = 7;  
        break;
    default:
        extr_len = 0;
        break;
    }

    uint64_t code = revCodes(CCL[key].code, CCL[key].len);

    if(bitpos + CCL[key].len > 64) flush(buffer, bitpos);
    buffer|= (code << bitpos);
    bitpos+=CCL[key].len;
    

    if(bitpos + extr_len > 64) flush(buffer, bitpos);
    code = extra;
    buffer|= (code<<bitpos);
    bitpos+=extr_len;
}

void buildTree(){
    using namespace bn_heap;

     //fix so value isnt uint and that it gets initial value /0 or smth different than -1 
    Node left = extrt();
    Node right = extrt();

    //add size to bnheap to fix random latent buggs
    //fix this either change uint32 to it so -1 does overflow or
    //add isleaf to node and it wont push and last
    //can add a size function to check before extracting
    while(left.value != -1 && right.value != -1){
        Node parent;
        tree.push_back(left);
        tree.push_back(right);

        parent.freq = left.freq + right.freq;
        parent.left = tree.size()-2;
        parent.right = tree.size()-1;

        insert(parent);

        left = extrt();
        right = extrt();
    }

    if(right.value == -1){
        tree.push_back(left);
    }
    //if(parent.left == -1){
    //    parent = left;
    //}
    //tree.push_back(parent); //root 

}

void createLen(std::vector<bits> &val_len, FLC *arr,  uint16_t len, int i){
    //len++;

    if(len > MAX_BITS) throw std::runtime_error("BITS are tooo long");

    if(tree[i].left == -1 && tree[i].right == -1){
        if(len == 0) len=1;
        arr[tree[i].value].len = len;
        val_len.push_back({tree[i].value, len});
        return;
    }

    if(tree[i].left >= 0){
        createLen(val_len, arr, len+1, tree[i].left);
    }
    if(tree[i].right >= 0){
        createLen(val_len, arr, len+1, tree[i].right);
    }
}

void createCodes(FLC *arr){
    std::vector<bits> value_len;
    
    //canonical might be broken fix it :/ what a bitch
    /*
    !!! IF NO LEN OR NO DIST THERS NO FUNCTION TO FIX IT
    !!! THE RESPONSE SHOULD BE THE FIRST SYMBOL HAS LENGHT 0
    !!! so if ARR is empty then HDIST = 1(1-1=0) and the next code is 0
    
    */

    createLen(value_len, arr, 0, tree.size()-1); //last element is root

    merge::sort(value_len,[](const bits &a, const bits &b){
        if(a.length == b.length)
            return a.value < b.value;
        return a.length < b.length;
    }); //ascending


    uint32_t code = 0;
    int prev_len = 0;

    for(bits &b : value_len){
        if(b.length > prev_len) code<<=(b.length-prev_len);
        arr[b.value].code = code++;
        prev_len = b.length;
    }

}

void processDictionaryRun(CCL_mode mode, uint64_t &buffer, int &bitpos){
    if(mode == WRITE){
        int prev = literal[0].len;
        int prev_count = 1;// change from 0 to 1

        for(int i=1; i<(HLIT+257); i++){
            if(prev == literal[i].len) prev_count++;
            else{
                outputDictionary(prev, prev_count, buffer, bitpos);
                prev = literal[i].len;
                prev_count = 1;
            }
        }

        for(int i=0; i<(HDIST+1); i++){
            if(prev==dist[i].len) prev_count++;
            else{ 
                outputDictionary(prev, prev_count, buffer, bitpos);
                prev = dist[i].len;
                prev_count = 1;
                continue;
            }
        }
        outputDictionary(prev, prev_count, buffer, bitpos);


    }else{
        
    int prev = literal[0].len;
    int prev_count=1;
    for(int i=1; i<maxCodeValue; i++){
        if(prev == literal[i].len) prev_count++;
        else{
            createCCLFreq(prev, prev_count);
            prev = literal[i].len;
            prev_count = 1;
        }
    }
    for(int i=0; i<maxDistValue; i++){
        if(prev == dist[i].len) prev_count++;
        else{
            createCCLFreq(prev, prev_count);
            prev = dist[i].len;
            prev_count = 1;
        }
    }
    createCCLFreq(prev, prev_count);
    }
}

uint64_t revCodes(uint64_t code, int len){
    uint64_t rev=0;
    while (len--)
    {
        rev<<=1;
        rev|= (code & 0b1);
        code>>=1;
    }
    return rev;
}

//only for huffman codes
void writeBitcode(uint64_t code, int len, uint64_t &buffer, int &bitpos){
    code = revCodes(code, len);
    if(bitpos + len > 64) flush(buffer, bitpos);
    buffer|= (code<<bitpos);
    bitpos+=len;
}

void writeExtra(int key, uint64_t &buffer, int &bitpos,  const bool len){
    //forgot i need to add the extra bits so more code and methods for dcode abd lcode
    uint64_t extra;
    int indx;

    if(len){
        indx = lcode(key);
        if(indx == -1) throw std::runtime_error("Invalid lenght");
        extra = lengthTable[indx].extraBits;
        if(extra+bitpos > 64) flush(buffer, bitpos);
        buffer|= (static_cast<uint64_t>(key-lengthTable[indx].baselength)<<bitpos);

    }else{
        indx = dcode(key);
        extra = distTable[indx].extraBits;
        if(extra+bitpos > 64) flush(buffer, bitpos);
        buffer|= (static_cast<uint64_t>(key-distTable[indx].basedist)<<bitpos);

    }
    bitpos+=extra;
}

void flush(uint64_t &buffer, int &bitpos, bool final){

    int bytes = bitpos/8; //bitpos starts with 0
    if(final && bitpos%8) bytes++;
    uint8_t *tmp = new uint8_t[bytes];

    for(int i=0; i<bytes; i++){
        tmp[i] = buffer & 0xFF;
        buffer>>=8;
    }
    for(int i=0; i<bytes; i++){
        printf("%02X", tmp[i]);
    }

    out.write(reinterpret_cast<char*>(tmp), bytes);
    bitpos-= bytes*8;
}

