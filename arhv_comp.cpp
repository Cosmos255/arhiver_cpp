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

int HLIT;
int HDIST;
int HCLEN;


std::vector<bn_heap::Node> tree;

std::unordered_map<int, uint64_t> codes; // the map for the literals and lengths
std::unordered_map<int, uint64_t> dist; //the map for the distances
std::unordered_map<int, uint64_t> CCL; 

void createHeap(int *arr, int size, std::vector<bn_heap::Node> &tree);

int main(){

    tree.reserve(1);

    auto lzed = lz77_token("file.example");
    
    for(token &tk : lzed){
        if(tk.type == match){
            code_freq[257 + lcode(tk.len)]++;
            dist_freq[dcode(tk.dist)]++;
        }
        else code_freq[(int)(tk.data)]++;

    }

    for(int i=0; i<maxCodeValue; i++)
    if(code_freq[i]){
        bn_heap::insert({i, code_freq[i]});
        HLIT++;
    } 
    buildTree();
    createCodes(code_len, codes);
    tree.clear();
    bn_heap::clear();
    
    for(int i=0; i<maxDistValue; i++)
    if(dist_freq[i]){
        bn_heap::insert({i, dist_freq[i]});
        HDIST++;
    } 
    buildTree();
    createCodes(dist_len, dist);
    tree.clear();
    bn_heap::clear();


    

    //Creating the header and blocks
    
    HLIT-=257;
    HDIST-=1;

    std::string buffer;

    uint64_t buffer = 0b010;




    return 0;
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
