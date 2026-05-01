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

    int buffer_space = 64;
    uint64_t buffer = 0b010;
    buffer_space-=3;


    bool compresed = true;
    bool dynamicHuffman = true;
    bool lastblock = true;

    if (lastblock) buffer |= 1;
    buffer<<=2;
    if(compresed){
        if(dynamicHuffman) buffer |= 0b01; //reversed cuz of LSB
        else buffer |= 0b10; //lsb
    }else{
        buffer|= 0b11;
    }

    //HLIT 5bits

    buffer-=5;
    for(int i=0; i<5; i++){
        buffer<<=1;
        buffer|= (HLIT & 0b1);
        HLIT>>=1;
    }

    //HDIST 5 bits

    buffer-=5;
    for(int i=0; i<5; i++){
        buffer<<=1;
        buffer|= (HDIST & 0b1);
        HDIST>>=1;
    }

    //HCLEN 4 bits

    buffer-=4;
    for(int i=0; i<5; i++){
        buffer<<=1;
        buffer|= (HCLEN & 0b1);
        HCLEN>>=1;
    }


    int CCL_order[] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    //CCL output

    for(int i=0; i<19; i++){
        int code = CCL[CCL_order[i]];
        for(int c=0; c<CCL_len[CCL_order[i]]; c++){
            buffer<<=1;
            buffer|= code & 0b1;
            code>>=1;
        }
    }


    //Dictionay output with ccl

    int prev = -1;
    int prev_count = 0;

    for(int i=0; i<maxCodeValue; i++){
        if(prev == code_len[i]) prev_count++;
        else{
            outputDictionary(prev, prev_count, buffer);
            prev = code_len[i];
        }
    }

    for(int i=0; i<maxDistValue; i++){
        if(prev==dist_len[i]) prev_count++;
        else{ 
            outputDictionary(prev, prev_count, buffer);
            prev = code_len[i];
            continue;
        }
        if(i == maxDistValue-1) outputDictionary(prev, prev_count, buffer); //for the dist bc if the last one is a match it wont output the last ones;
    }


    //outputting the compressed data





    return 0;



    //now its decompresser time
}

void outputDictionary(const int prev, int &prev_count, uint64_t &buffer){
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

    buffer<<=extr_len;
    buffer|=extra;

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