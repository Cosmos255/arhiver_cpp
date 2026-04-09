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
constexpr int maxCodeValue = 257;

int value_freq[maxCodeValue] = {0};


int bl_count[maxCodeValue];



int len[deflateBitLength] = {0};
int dist[deflateBitDist] = {0};


std::vector<bn_heap::Node> tree;


//can be replaced with a node struct which use indices instead of pointers so it will be 1 or 2 big arrays
std::vector<std::unique_ptr<Tree>> tree_list;

void createHeap(int *arr, int size, std::vector<bn_heap::Node> &tree);

int main(){

    tree.reserve(1);

    auto lzed = lz77_token("file.example");
    
    for(token &tk : lzed){
        if(tk.type == match) value_freq[256]++;
        else value_freq[(int)(tk.data)]++;

        /*
        //for dynamic huffman
        len[lcode(tk.len)]++;
        dist[dcode(tk.dist)]++;
        */
        /*
        len[(int)(tk.len)]++;
        dist[(int)(tk.dist)]++;
        */
    }
    int arr_size = 257;
    for(int i=0; i<arr_size; i++)
    if(value_freq[i] >= 1) bn_heap::insert({i, value_freq[i]});
    
    buildTree(tree);

    //merge::sort(tree_list, []( const bn_heap::Node &a, const bn_heap::Node &b ){return a.freq < b.freq;});

    
    return 0;
}


void buildTree(std::vector<bn_heap::Node> &tree){
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


void createCodes(std::vector<bn_heap::Node> &tree){

}

/*
void BuildTree(std::vector<Node> &tree){
    int i = tree.size()-1;

    while(i){
        tree[i-2].left = i-1;
        tree[i-2].right = i;
        if(i>=2) i-=2;
        else{
            tree[0].right = 1;
        }
    }

}
*/
///greedy recursiv pentru fiecare cautam cel mai mic destul de usor +- idk coaie