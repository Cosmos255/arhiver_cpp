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

int HLIT=0;
int HDIST=0;
int HCLEN=0;

std::vector<bn_heap::Node> tree;

std::unordered_map<int, uint64_t> codes; // the map for the literals and lengths
std::unordered_map<int, uint64_t> dist; //the map for the distances
std::unordered_map<int, uint64_t> CCL; //these 3 maps can be replaced with 3 simple arrays as the amount of values is fixed



void outputDictionary(const int prev, int &prev_count, uint64_t &buffer, int &bitpos);
void outputCCLCode(int key, int &count, uint64_t &buffer, int &bitpos);
void buildTree();
void createLen(std::vector<bits> &val_len, int *len_arr,  uint16_t len=0, int i=0);
void createCodes(int *len_arr, std::unordered_map<int, uint64_t> &map);
void createCCL_freq();
uint64_t revCodes(uint64_t code, int len);
void writeBitcode(uint64_t code, int len, uint64_t &buffer, int &bitpos);
void writeExtra(int key, uint64_t &buffer, int &bitpos,  const bool len);
void flush(uint64_t &buffer, int &bitpos);


std::ofstream out("out.txt", std::ios::out | std::ios::binary);

int main(){
    if (!out) {
        std::cerr << "Failed to open file\n";
    }

    tree.reserve(1); //should add smth to improve performance

    auto lzed = lz77_token("example.txt");
    
    //creating a frequency array from tokens
    for(token &tk : lzed){
        if(tk.type == match){
            code_freq[257 + lcode(tk.len)]++;
            dist_freq[dcode(tk.dist)]++;
        }
        else code_freq[(int)(tk.data)]++;

    }

    //literal length codes
    for(int i=0; i<maxCodeValue; i++)
    if(code_freq[i]){
        bn_heap::insert({i, code_freq[i]});
        HLIT++;
    } 
    buildTree();

    std::cout<<"\n Freq"<<tree[tree.size()-1].freq<<"\n";
    std::cout<<"\n LEFT"<<tree[tree.size()-1].left<<"\n";
    std::cout<<"\n RIGHT"<<tree[tree.size()-1].right<<"\n";
    std::cout<<"\n VALUE"<<tree[tree.size()-1].value<<"\n";
    createCodes(code_len, codes);
    tree.clear();
    bn_heap::clear();
    
    //dist codes
    for(int i=0; i<maxDistValue; i++)
    if(dist_freq[i]){
        bn_heap::insert({i, dist_freq[i]});
        HDIST++;
    } 
    buildTree();
    createCodes(dist_len, dist);
    tree.clear();
    bn_heap::clear();


    createCCL_freq();


    for(int i=0; i<19; i++)
        if(CCL_freq[i]){
            bn_heap::insert({i, CCL_freq[i]});
            HCLEN++;
    }
    
    buildTree();
    createCodes(CCL_len, CCL);
    tree.clear();
    bn_heap::clear();


    //Creating the header and blocks and outputtign the data
    
    HLIT-=257;
    HDIST-=1;
    HCLEN-=4;

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

    buffer |= (static_cast<uint64_t>(HLIT) << bitpos);
    bitpos+=5;


    //HDIST 5 bits

    buffer |= (static_cast<uint64_t>(HDIST) << bitpos);
    bitpos+=5;

    //HCLEN 4 bits

    buffer |= (static_cast<uint64_t>(HCLEN) << bitpos);
    bitpos+=4;


    //CCL output



    for(int i=0; i<19; i++){
        uint64_t code = revCodes(CCL[CCL_order[i]], CCL_len[CCL_order[i]]); //maybe use key=CCL_order[i]
        if(bitpos + CCL_len[CCL_order[i]] > 63) flush(buffer, bitpos);
        buffer|=(code << bitpos);
        bitpos+=CCL_len[CCL_order[i]];

    }


    //Dictionay output with ccl

    int prev = code_len[0];
    int prev_count = 0;

    for(int i=1; i<maxCodeValue; i++){
        if(prev == code_len[i]) prev_count++;
        else{
            outputDictionary(prev, prev_count, buffer, bitpos);
            prev = code_len[i];
        }
    }

    for(int i=0; i<maxDistValue; i++){
        if(prev==dist_len[i]) prev_count++;
        else{ 
            outputDictionary(prev, prev_count, buffer, bitpos);
            prev = dist_len[i];
            continue;
        }
        if(i == maxDistValue-1) outputDictionary(prev, prev_count, buffer, bitpos); //for the dist bc if the last one is a match it wont output the last ones;
    }


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

    char x = 'A';
    out.write(&x, 1);
    out.close();

    out.flush();
    out.close();

    return 0;



    //now its decompresser time
}

void outputDictionary(const int prev, int &prev_count, uint64_t &buffer, int &bitpos){
    outputCCLCode(prev, prev_count, buffer, bitpos);

    if(prev > 0) while(prev_count >= 3) outputCCLCode(16, prev_count, buffer, bitpos);    

    if(prev == 0){
        while(prev_count > 10){
            outputCCLCode(18, prev_count, buffer, bitpos);
        }
        while (prev_count < 11 && prev_count >= 3){
            outputCCLCode(17, prev_count, buffer, bitpos);
        }
    }

    while(prev_count > 0){
        outputCCLCode(prev, prev_count, buffer, bitpos);
        prev_count--;
    }

    prev_count = 0;
}

void outputCCLCode(int key, int &count, uint64_t &buffer, int &bitpos){
    int extra = 0;
    int extr_len = 2;
    uint64_t code;
    switch (key){
    case 16:
        extra = (count%6)-3;
        count-=6;
        extr_len = 2;
        break;
    case 17:
        extra = (count%10)-3;
        count-=10;
        extr_len = 3;
        break;
    case 18:
        extra = (count%138)-11;
        count-=138;  
        extr_len = 7;  
        break;
    default:
        break;
    }
    code = revCodes(CCL[key], CCL_len[key]);

    if(bitpos + CCL_len[key] > 63) flush(buffer, bitpos);
    buffer|= (code << bitpos);
    bitpos+=CCL_len[key];
    
    if(bitpos + extr_len > 63) flush(buffer, bitpos);
    code = revCodes(extra, extr_len);
    buffer|= (code<<bitpos);
    bitpos+=extr_len;

    count = std::max(0, count);
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

    if(parent.left == -1){
        parent = left;
    }
    tree.push_back(parent); //root 

}

void createLen(std::vector<bits> &val_len, int *len_arr,  uint16_t len, int i){

    len++;
    if(tree[i].left == -1 && tree[i].right == -1){
        len_arr[tree[i].value] = len;
        val_len.push_back({static_cast<uint32_t>(tree[i].value), len});
        return;
    }

    if(tree[i].left){
        createLen(val_len, len_arr, len, tree[i].left);
    }
    if(tree[i].right){
        createLen(val_len, len_arr, len, tree[i].right);
    }
}

void createCodes(int *len_arr, std::unordered_map<int, uint64_t> &map){
    std::vector<bits> value_len;
    
    createLen(value_len, len_arr, 0, tree.size()-1); //last element is root

    merge::sort(value_len,[](const bits &a, const bits &b){return a.length < b.length;}); //ascending

    int i=1;
    while(i < value_len.size()){
        if(value_len[i-1].length == value_len[i].length){
            if(value_len[i-1].value > value_len[i].value) std::swap(value_len[i], value_len[i-1]);
        }
        i++;
    } //sorting alphabeticly

    //building the codes

    
    map[value_len[0].value] = 0;
    int prev_length = value_len[0].length;
    uint32_t code = 0;

    i = 1;

    while(i < value_len.size()){
        bits current = value_len[i];
        code++;
        if(current.length > prev_length) code<<= current.length-prev_length;
        map[current.value] = code;
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

void flush(uint64_t &buffer, int &bitpos){
    int bytes = (bitpos+1)/8; //bitpos starts with 0

    out.write(reinterpret_cast<const char*>(&buffer), bytes); //should work
    bitpos-= (bytes*8)-1;
    buffer>>=bytes*8;
}