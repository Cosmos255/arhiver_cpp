#include "../include/utils.hpp"
#include "../include/lz77.hpp"
#include <cstdio>
#include <assert.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>
#include <exception>

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






std::ofstream out("out.bin", std::ios::out | std::ios::trunc | std::ios::binary);

constexpr int OUTBUFFER_SIZE = 1<<20; //1MB
std::vector<uint8_t> outbuffer(OUTBUFFER_SIZE);
int outbuff_indx = 0;

    
uint64_t buffer = 0;
int bitpos = 0; //max is 64

int main(){
    try{
        
    }catch(const std::exception& e){
        std::cerr<<e.what()<<"\n";
        return 1; //error
    }

    bool distance = false;

    if (!out) {
        std::cerr << "Failed to open file\n";
    }

    //tree.reserve(600); //should add smth to improve performance

    auto lzed = lz77_token("example.txt");//tokens from lz77 algo


    for(token &tk : lzed){
        if(tk.type == match){
            if(lcode(tk.len) < 0) throw std::runtime_error("Lcode is negative");
            if(dcode(tk.dist) < 0) throw std::runtime_error("Dcode is negative");

            literal[257 + lcode(tk.len)].freq++;
            dist[dcode(tk.dist)].freq++;
            distance = true;
        }
        else literal[(uint8_t)(tk.data)].freq++;

    }


    literal[256].freq++;//end of block

    //literal length codes
    
    for(int i=0; i<maxCodeValue; i++)
    if(literal[i].freq){
        bn_heap::insert({i, literal[i].freq});
    }



    buildTree(); //treebuilder from bn heap
    createCodes(literal); //creates the huffman codes
    tree.clear();
    bn_heap::clear();

    if(distance){

    //dist codes
        for(int i=0; i<maxDistValue; i++)
        if(dist[i].freq){
            bn_heap::insert({i, dist[i].freq});
        } 

        buildTree();
        createCodes(dist);
        tree.clear();
        bn_heap::clear();
    
    }else dist[0].len = 0;
    

    HLIT = maxCodeValue;
    HDIST = maxDistValue;


    while(HLIT >= 256 && literal[HLIT-1].len == 0) HLIT--;
    if(HLIT < 257) throw std::runtime_error("HLIT lower than 256");

    if(!distance) HDIST = 1;
    else while(HDIST > 0 && dist[HDIST-1].len == 0) HDIST--;

    processDictionaryRun();//coounts freq for repeatcodes and creates a vector for output

    for(int i=0; i<19; i++)
        if(CCL[i].freq){
            bn_heap::insert({i, CCL[i].freq});
    }
    
    buildTree(); //tree for CCL
    createCodes(CCL); //codes for CCL
    tree.clear();
    bn_heap::clear();


    //Creating the header and blocks and outputtign the data

    HCLEN = 19;

    while(HCLEN >= 4 && CCL[CCL_order[HCLEN-1]].len == 0) HCLEN--;
    if(HCLEN < 4) throw std::runtime_error("HCLEN lower than 4");

    HLIT-=257;
    HDIST-=1;
    HCLEN-=4;


    if(HLIT < 0 || HDIST < 0 || HCLEN < 0) throw std::runtime_error("Unexpected negative values");


    //add seting for stored and type of compression

    bool compresed = true;
    bool dynamicHuffman = true;
    bool lastblock = true;

    if (lastblock) writeBits(1, 1);
    else writeBits(0, 1);

    if(compresed){
        if(dynamicHuffman) writeBits(0b10, 2);
        else writeBits(0b01, 2);
    }else{
        writeBits(0, 2);
    }





    //add static cast uint64t to all codes that are written into the buffer
    //cuz its done in int space not uint


    //HLIT 5bits
    writeBits(HLIT, 5);

    //HDIST 5 bits
    writeBits(HDIST, 5);

    //HCLEN 4 bits
    writeBits(HCLEN, 4);


    std::cout<<"HLIT: "<<HLIT+257;
    std::cout<<"HDIST: "<<HDIST+1;
    std::cout<<"HCLEN: "<<HCLEN+4<<"\n";


    //CCL output

    for(int i=0; i<(HCLEN+4); i++){
        int key = CCL_order[i];
        if(CCL[key].len > 7) throw std::runtime_error("CCL key too long");
        writeBits((CCL[key].len & 0b111), 3);
    }

    outputDictionary(); //outputs the dictionary


    for(token &t : lzed){
        if(t.type == match){
            writeBitcode(literal[257 + lcode(t.len)].code, literal[257 + lcode(t.len)].len); //wait what about dcode and lcode ?? 
            writeExtra(t.len, true);
            writeBitcode(dist[dcode(t.dist)].code, dist[dcode(t.dist)].len);
            writeExtra(t.dist, false);
        }
        else{
            writeBitcode(literal[t.data].code, literal[t.data].len);
        }
    }

    writeBitcode(literal[256].code, literal[256].len); //this should be it i think
    flush(1);

    out.close();
    return 0;

    std::cerr<<"THE HDIST HCLEN and HLIN arent computed correctly so it doesnt work";


}



void buildTree(){
    using namespace bn_heap;

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
        //might be a mistak
        tree.push_back(left);
    }
    //th
}

void createLen(std::vector<bits> &val_len, FLC *arr, int i, uint16_t len = 1){

    if(len > MAX_BITS) throw std::runtime_error("BITS are tooo long");

    if(tree[i].left == -1 && tree[i].right == -1){
        arr[tree[i].value].len = len;
        val_len.push_back({tree[i].value, len});
        return;
    }

    if(tree[i].left >= 0){
        createLen(val_len, arr, tree[i].left, len+1);
    }
    if(tree[i].right >= 0){
        createLen(val_len, arr,tree[i].right, len+1);
    }
}


void createCodes(FLC *arr = nullptr){
    if(arr == nullptr) throw std::runtime_error("createCode received nullptr");
    std::vector<bits> code_lengths;
    
    //canonical might be broken fix it :/ what a bitch
    /*
    !!! IF NO LEN OR NO DIST THERS NO FUNCTION TO FIX IT
    !!! THE RESPONSE SHOULD BE THE FIRST SYMBOL HAS LENGHT 0
    !!! so if ARR is empty then HDIST = 1(1-1=0) and the next code is 0
    
    */

    createLen(code_lengths, arr,tree.size()-1); //last element is root

    merge::sort(code_lengths,[](const bits &a, const bits &b){
        if(a.length == b.length)
            return a.value < b.value;
        return a.length < b.length;
    }); //ascending


    int len=0;
    int code=0;

    for(auto &bts : code_lengths){
        if(bts.length == 0) continue;
        assert(len <= 15);
        if(bts.length > len) code<<=(bts.length-len);
        len = bts.length;
        assert(len > 0);
        arr[bts.value].code = code++;

    }

}


struct lenCodes{
    int symbol = 0;
    int extra = 0;
};

std::vector<lenCodes> dictionary;

void outputDictionary(){
    for(auto &obj : dictionary){
        int extra_len = 0;
        int key = obj.symbol;

        switch (key){
        case 16:
            extra_len = 2;
            break;
        case 17:
            extra_len = 3;
            break;
        case 18:
            extra_len = 7;  
            break;
        default:
            extra_len = 0;
            break;
        }

        writeBitcode(CCL[key].code, CCL[key].len);
        if(extra_len) writeBits(obj.extra, extra_len);
    }
}


void createCCLFreq(const int prev, int &prev_count){
    int repeat_len=0;
    int extra=0;

    dictionary.push_back({prev, 0});
    CCL[prev].freq++;

    if(prev > 0) while(prev_count >= 3){
        repeat_len = std::min(prev_count, 6);
        extra = repeat_len-3;
        CCL[16].freq++;
        prev_count-=repeat_len;
        dictionary.push_back({16,extra});
    }
   

    if(prev == 0){
        while(prev_count >= 11){
            repeat_len = std::min(prev_count, 138); 
            extra = repeat_len - 11;
            CCL[18].freq++;
            prev_count-=repeat_len;
            dictionary.push_back({18, extra});
        }
        while (prev_count >= 3){
            repeat_len = std::min(prev_count, 10);
            extra = repeat_len - 3;
            CCL[17].freq++;
            prev_count-=repeat_len;
            dictionary.push_back({17, extra});
        }
    }



    CCL[prev].freq += prev_count;
    while(prev_count > 0){
        dictionary.push_back({prev, 0}); 
        prev_count--;
    }
    prev_count = 0;

}

void processDictionaryRun(){
    int prev =0;
    int prev_count=0;
    for(int i=1; i<HLIT; i++){
        prev = literal[i-1].len;
        if(prev == literal[i].len) prev_count++;
        else{
            createCCLFreq(prev, prev_count);
            prev_count = 0;
        }
    }

    prev = literal[HLIT-1].len;

    //createCCLFreq(prev, prev_count);

    for(int i=0; i<HDIST; i++){
        if(prev == dist[i].len) prev_count++;
        else{
            createCCLFreq(prev, prev_count);
            prev_count = 0;
            prev = dist[i].len;
        }
    }
    
    createCCLFreq(prev, prev_count);

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
void writeBitcode(uint64_t code, int len){
    code = revCodes(code, len);
    if(bitpos + len > 64) flush();
    buffer|= (code<<bitpos);
    bitpos+=len;
}

void writeExtra(int key, const bool len){
    //forgot i need to add the extra bits so more code and methods for dcode abd lcode
    uint64_t extra;
    int indx;

    if(len){
        indx = lcode(key);
        if(indx == -1) throw std::runtime_error("Invalid lenght");
        extra = lengthTable[indx].extraBits;
        if(extra+bitpos > 64) flush();
        buffer|= (static_cast<uint64_t>(key-lengthTable[indx].baselength)<<bitpos);

    }else{
        indx = dcode(key);
        extra = distTable[indx].extraBits;
        if(extra+bitpos > 64) flush();
        buffer|= (static_cast<uint64_t>(key-distTable[indx].basedist)<<bitpos);

    }
    bitpos+=extra;
}


void writeBits(uint64_t code, int len){
    if(bitpos + len > 64) flush();
    buffer|= (code<<bitpos);
    bitpos+=len;
}

void flush(bool forced){
    int rm = 0;
    if(forced) rm = (bitpos + 7) & ~7;
    else rm = bitpos - (bitpos%8);

    assert(rm > 0);
    bitpos-=rm;

    if(rm > (outbuffer.size()-outbuff_indx)){
        out.write(reinterpret_cast<char *>(outbuffer.data()), outbuff_indx);
        outbuff_indx = 0;
    }

    while(rm){
        outbuffer[outbuff_indx] = buffer & 0xFF;
        buffer>>=8;
        outbuff_indx++;
        rm-=8;
    }
    if(forced) out.write(reinterpret_cast<char *>(outbuffer.data()), outbuff_indx);
}