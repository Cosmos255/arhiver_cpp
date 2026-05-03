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


int main(){

    tree.reserve(1); //should add smth to improve performance

    auto lzed = lz77_token("file.example");
    
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

        buffer|=(code << bitpos);
        bitpos+=CCL_len[CCL_order[i]];

    }


    //Dictionay output with ccl

    int prev = -1;
    int prev_count = 0;

    for(int i=0; i<maxCodeValue; i++){
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
            prev = code_len[i];
            continue;
        }
        if(i == maxDistValue-1) outputDictionary(prev, prev_count, buffer, bitpos); //for the dist bc if the last one is a match it wont output the last ones;
    }


    //outputting the compressed data

    for(token &t : lzed){
        if(t.type == match){
            writeBitcode(t.len, codes, );
            writeExtra();
            writeBitcode();
            writeExtra();
        }
        else{
            writeBitcode();
        }
    }

    writeBitcode();
    writeExtra();



    return 0;



    //now its decompresser time
}

void outputDictionary(const int prev, int &prev_count, uint64_t &buffer, int bitpos){
    outputCCLCode(prev, prev_count, buffer);

    if(prev > 0) while(prev_count >= 3) outputCCLCode(16, prev_count, buffer);    


    if(prev == 0){
        while(prev_count > 10){
            outputCCLCode(18, prev_count, buffer);
        }
        while (prev_count < 11 && prev_count >= 3){
            outputCCLCode(17, prev_count, buffer);
        }
    }

    while(prev_count--){
        outputCCLCode(prev, prev_count, buffer);
    }

    prev_count = 0;
}

void outputCCLCode(int key, int &count, uint64_t &buffer){
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
    code = CCL[key];

    for(int i=0; i<CCL_len[key]; i++){
        buffer<<=1;
        buffer|= (code & 1);
        code>>=1;
    }
    for(int i=1; i<=extr_len; i++){
        buffer<<=i;
        buffer|= extra & 0b1;
        extra>>=1;
    }
}

void buildTree(){
    using namespace bn_heap;

    Node parent;
    Node left = extrt();
    Node right = extrt();

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

void createLen(std::vector<bits> &val_len, int *len_arr,  uint16_t len=0, int i){

    len++;
    if(tree[i].left){
        createLen(val_len, len_arr, len, tree[i].left);
    }
    if(tree[i].right){
        createLen(val_len, len_arr, len, tree[i].right);
    }
    if(tree[i].left == -1 && tree[i].right == -1){
        len_arr[tree[i].value] = len;
        val_len.push_back({tree[i].value, len});
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

    int i = 1;

    while(i < value_len.size()){
        bits current = value_len[i];
        code++;
        if(current.length > prev_length) code<<= current.length-prev_length;
        map[current.value] = code;
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

void writeBitcode(uint32_t code, int len, strctBuff buf){
    if(buf.bitpose + len > 64) flush();
    buf.buffer|= (code<<buf.bitpose);
    buf.bitpose+=len;
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



//Rewrite/fix writeBitcode writeExtra and flush

void writeBitcode(int index, std::unordered_map<int, uint64_t> map, int &freebits, uint64_t &outBuffer){    
    uint64_t disp;

    disp = map[index].length;
    if(disp > freebits) flush(outBuffer, freebits);
    outBuffer = (outBuffer<<disp) | map[index].bits;
    freebits-=disp;
        
    }

void writeExtra(int value, int &freebits, uint64_t &outBuffer, const bool len){
    //forgot i need to add the extra bits so more code and methods for dcode abd lcode
    uint64_t disp;
    int indx;

    if(len){
        indx = lcode(value);
        disp = lengthTable[indx].extraBits;
        if(disp > freebits) flush(outBuffer, freebits);
        outBuffer = (outBuffer<<disp) | (value - lengthTable[indx].baselength);

    }else{
        indx = dcode(value);
        disp = distTable[indx].extraBits;
        if(disp > freebits) flush(outBuffer, freebits);
        outBuffer = (outBuffer<<disp) | (value - distTable[indx].basedist);
    }

    freebits-=disp;

}

void flush(uint64_t &buffer, int &freebits){

    int bytes = (64-freebits) / 8; 
    //replace_this.write(reinterpret_cast<char*>(buffer>>(8-bytes)*8), bytes); //replace with string.data so its safer and works
    replace_this.write(reinterpret_cast<char*>(&buffer), bytes); //should work also this write bottom -> up buffer 
    freebits+= bytes*8;
    //good for stirng buffer = ((buffer<<bytes*8) & 0xFFFFFFFFFFFFFFFF)>>bytes*8; //remove the top x bytes
    buffer <<=bytes*8;
    buffer >>=bytes*8;
}