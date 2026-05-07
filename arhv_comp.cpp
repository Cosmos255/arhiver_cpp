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

//FLC codes[maxCodeValue] = {};
//FLC dist[maxDistValue] = {};
//FLC CCL[19];

//using CodeSize = uint64_t; //just for testing rn

const int MAX_BITS = 15;

constexpr int maxCodeValue = 286;
constexpr int maxDistValue = 30;


//int code_len[maxCodeValue] = {0};
int code_len[maxCodeValue] = {0};
int dist_len[maxDistValue] = {0};
int CCL_len[19] = {0};

int code_freq[maxCodeValue] = {0};
int dist_freq[maxDistValue] = {0};
int CCL_freq[19]=  {0};

int HLIT=257;
int HDIST=1;
int HCLEN=4;

std::vector<bn_heap::Node> tree;

std::unordered_map<int, uint64_t> codes; // the map for the literals and lengths
std::unordered_map<int, uint64_t> dist; //the map for the distances
std::unordered_map<int, uint64_t> CCL; //these 3 maps can be replaced with 3 simple arrays as the amount of values is fixed



void outputDictionary(const int prev, int &prev_count, uint64_t &buffer, int &bitpos);
void outputCCLCode(int key, int extra, uint64_t &buffer, int &bitpos);
void buildTree();
void createLen(std::vector<bits> &val_len, int *len_arr,  uint16_t len=0, int i=0);
void createCodes(int *len_arr, std::unordered_map<int, uint64_t> &map);
void createCCL_freq();
uint64_t revCodes(uint64_t code, int len);
void writeBitcode(uint64_t code, int len, uint64_t &buffer, int &bitpos);
void writeExtra(int key, uint64_t &buffer, int &bitpos,  const bool len);
void flush(uint64_t &buffer, int &bitpos, bool final=0);


std::ofstream out("out.txt", std::ios::binary);

int main(){
    if (!out) {
        std::cerr << "Failed to open file\n";
    }

    tree.reserve(600); //should add smth to improve performance

    auto lzed = lz77_token("example.txt");
    
    //creating a frequency array from tokens
    for(token &tk : lzed){
        if(tk.type == match){
            code_freq[257 + lcode(tk.len)]++;
            dist_freq[dcode(tk.dist)]++;
        }
        else code_freq[(uint8_t)(tk.data)]++;

    }
    for(int i=0; i<maxDistValue; i++){
        if(dist_freq) break;
        if(i == maxDistValue-1) code_freq[0] = 1;
    }

    code_freq[256]++; //for end of block

    //literal length codes
    
    for(int i=0; i<maxCodeValue; i++)
    if(code_freq[i]){
        bn_heap::insert({i, code_freq[i]});
        
    } 
    buildTree();
    createCodes(code_len, codes);
    tree.clear();
    bn_heap::clear();
    
    //dist codes
    for(int i=0; i<maxDistValue; i++)
    if(dist_freq[i]){
        bn_heap::insert({i, dist_freq[i]});
    } 
    buildTree();
    createCodes(dist_len, dist);
    tree.clear();
    bn_heap::clear();


    createCCL_freq();


    for(int i=0; i<19; i++)
        if(CCL_freq[i]){
            bn_heap::insert({i, CCL_freq[i]});
    }
    
    buildTree();
    createCodes(CCL_len, CCL);
    tree.clear();
    bn_heap::clear();


    //Creating the header and blocks and outputtign the data

    HLIT = maxCodeValue-1;
    while(HLIT > 257 && code_len[HLIT] == 0){
        HLIT--;
    }

    HDIST = maxDistValue-1;
    while(HDIST > 1 && dist_len[HDIST] == 0){
        HDIST--;
    }

    HCLEN = 18;
    while (HCLEN > 4 && CCL_len[CCL_order[HCLEN]] == 0){
        HCLEN--;
    }
    
    HLIT-=257;
    HDIST-=1;
    HCLEN-=4;

    if(HLIT < 0 || HDIST < 0 || HCLEN < 0) throw std::runtime_error("Unexpected negative values");
    

    //std::string buffer;

    uint64_t buffer = 0;
    int bitpos = 0; //max is 64


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

    buffer |= (static_cast<uint8_t>(HLIT) << bitpos);
    bitpos+=5;


    //HDIST 5 bits

    buffer |= (static_cast<uint8_t>(HDIST) << bitpos);
    bitpos+=5;

    //HCLEN 4 bits

    buffer |= (static_cast<uint8_t>(HCLEN) << bitpos);
    bitpos+=4;


    //CCL output



    for(int i=0; i<=(HCLEN+4); i++){
        uint64_t code = revCodes(CCL[CCL_order[i]], CCL_len[CCL_order[i]]); //maybe use key=CCL_order[i]
        if(bitpos + CCL_len[CCL_order[i]] > 64) flush(buffer, bitpos);
        buffer|=(code << bitpos);
        bitpos+=CCL_len[CCL_order[i]];

    }


    //Dictionay output with ccl

    int prev = code_len[0];
    int prev_count = 0;

    for(int i=1; i<=(HLIT+257); i++){
        if(prev == code_len[i]) prev_count++;
        else{
            outputDictionary(prev, prev_count, buffer, bitpos);
            prev = code_len[i];
            prev_count = 1;
        }
    }
    outputDictionary(prev, prev_count, buffer, bitpos);

    prev = dist_len[0];
    prev_count = 0;

    for(int i=0; i<=(HDIST+1); i++){
        if(prev==dist_len[i]) prev_count++;
        else{ 
            outputDictionary(prev, prev_count, buffer, bitpos);
            prev = dist_len[i];
            prev_count = 1;
            continue;
        }
    }
    outputDictionary(prev, prev_count, buffer, bitpos);

    //outputting the compressed data

    for(token &t : lzed){
        if(t.type == match){
            writeBitcode(codes[t.len], code_len[t.len], buffer, bitpos); //i think it works but i am not sure
            writeExtra(t.len, buffer, bitpos, true);
            writeBitcode(dist[t.dist], dist_len[t.dist], buffer, bitpos);
            writeExtra(t.dist, buffer, bitpos, false);
        }
        else{
            writeBitcode(codes[t.data], code_len[t.data], buffer, bitpos);
        }
    }

    writeBitcode(codes[256], code_len[256], buffer, bitpos); //this should be it i think
    flush(buffer, bitpos, 1);

    out.close();
    return 0;

    std::cerr<<"THE HDIST HCLEN and HLIN arent computed correctly so it doesnt work";

    //now its decompresser time
}

void outputDictionary(const int prev, int &prev_count, uint64_t &buffer, int &bitpos){
     
    outputCCLCode(prev, 0, buffer, bitpos);

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

    uint64_t code = revCodes(CCL[key], CCL_len[key]);

    if(bitpos + CCL_len[key] > 64) flush(buffer, bitpos);
    buffer|= (code << bitpos);
    bitpos+=CCL_len[key];
    

    if(bitpos + extr_len > 64) flush(buffer, bitpos);
    code = extra;
    buffer|= (code<<bitpos);
    bitpos+=extr_len;
}

void buildTree(){
    using namespace bn_heap;

    Node parent; //fix so value isnt uint and that it gets initial value /0 or smth different than -1 
    Node left = extrt();
    Node right = extrt();

    //add size to bnheap to fix random latent buggs
    //fix this either change uint32 to it so -1 does overflow or
    //add isleaf to node and it wont push and last
    //can add a size function to check before extracting

    while(left.value != -1 && right.value != -1){
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
        parent = left;
    }
    //if(parent.left == -1){
    //    parent = left;
    //}
    tree.push_back(parent); //root 

}

void createLen(std::vector<bits> &val_len, int *len_arr,  uint16_t len, int i){

    len++;
    if(tree[i].left == -1 && tree[i].right == -1){
        len_arr[tree[i].value] = len;
        val_len.push_back({static_cast<uint32_t>(tree[i].value), len});
        return;
    }

    if(tree[i].left >= 0){ //might be 0 is valid too 
        createLen(val_len, len_arr, len, tree[i].left);
    }
    if(tree[i].right >= 0){ // 0 might be valid too
        createLen(val_len, len_arr, len, tree[i].right);
    }
}

void createCodes(int *len_arr, std::unordered_map<int, uint64_t> &map){
    std::vector<bits> value_len;
    
    //canonical might be broken fix it :/ what a bitch

    createLen(value_len, len_arr, 0, tree.size()-1); //last element is root

    merge::sort(value_len,[](const bits &a, const bits &b){return a.length < b.length;}); //ascending

    bool changed = true;
    while (changed)
    {
        int i=1;
        changed = false;
        while(i < value_len.size()){

            if(value_len[i-1].length != value_len[i].length){
                i++;
                continue;
            }
            
            if(value_len[i-1].value > value_len[i].value){
                std::swap(value_len[i], value_len[i-1]);
                changed=true;
            }
            i++;
        }
    }
     //sorting alphabeticly

    //building the codes

    
    map[value_len[0].value] = 0;
    int prev_length = value_len[0].length;
    uint32_t code = 0;

    int i = 1;

    while(i < value_len.size()){
        bits current = value_len[i];
        code++;
        if(current.length > prev_length) code<<= current.length-prev_length;
        map[current.value] = code;
        prev_length = current.length;
        i++;
    }
}

void createCCL_freq(){
    int prev = -1;
    int prev_count=0;

    for(int i=0; i<maxCodeValue; i++){
        if(prev == code_len[i]) prev_count++;
        else{
            if(prev != 0){
                while(prev_count >= 3){
                    CCL_freq[16]++;
                    prev_count-=6;
                }
            }

            if(prev == 0){
                while(prev_count >=11){
                    CCL_freq[18]++;
                    prev_count-=138;
                }
                while (prev_count <= 10 && prev_count >=3){
                    CCL_freq[17]++;
                    prev_count-=10;
                }
            }
            
            if(prev_count > 0){
                CCL_freq[prev] += prev_count;
                prev = code_len[i];
                prev_count=0;
                continue;
            }  
        }

    }

    for(int i=0; i<maxDistValue; i++){
        if(prev == dist_len[i]) prev_count++;
        else{
            if(prev != 0){
                while(prev_count >= 3){
                    CCL_freq[16]++;
                    prev_count-=6;
                }
            }

            if(prev == 0){
                while(prev_count >=11){
                    CCL_freq[18]++;
                    prev_count-=138;
                }
                while (prev_count <= 10 && prev_count >=3){
                    CCL_freq[17]++;
                    prev_count-=10;
                }
            }
            
            if(prev_count > 0){
                CCL_freq[prev] += prev_count;
                prev = dist_len[i];
                prev_count=0;
                continue;
            }  
        }

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
        extra = lengthTable[indx].extraBits;
        if(extra+bitpos > 63) flush(buffer, bitpos);
        buffer|= ((key-lengthTable[indx].baselength)<<bitpos);

    }else{
        indx = dcode(key);
        extra = distTable[indx].extraBits;
        if(extra+bitpos > 63) flush(buffer, bitpos);
        buffer|= ((key-distTable[indx].basedist)<<bitpos);

    }
    bitpos+=extra;
}
/*
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
*/
void flush(uint64_t &buffer, int &bitpos, bool final) {
    int bytes = bitpos / 8;

    if (final && (bitpos % 8)) {
        bytes++; // include partial byte
    }

    uint8_t outbuf[8]; // max 64 bits = 8 bytes

    uint64_t temp = buffer; // copy ONLY for reading

    for (int i = 0; i < bytes; i++) {
        outbuf[i] = temp & 0xFF;
        temp >>= 8;
    }

    out.write((char*)outbuf, bytes);

    // 🔥 IMPORTANT PART:
    int remainingBits = bitpos - bytes * 8;

    if (remainingBits > 0) {
        // keep leftover bits WITHOUT shifting whole buffer incorrectly

        buffer >>= (bytes * 8);  // move consumed bits down
        buffer &= ((1ULL << remainingBits) - 1); // clean upper garbage
    } else {
        buffer = 0;
    }

    bitpos = remainingBits;
}