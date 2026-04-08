#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>
#include "utils.h"
#include "lz77.h"


const int MAX_BITS = 15;
constexpr int maxCodeValue = 257;

int value_freq[maxCodeValue] = {0};


int bl_count[maxCodeValue];



int len[deflateBitLength] = {0};
int dist[deflateBitDist] = {0};

struct Node{
    int data = '\0';
    int freq;
    int parent_freq = 0;
    int length; //code length
    int left = -1; //if -1 non existent
    int right = -1;

    Node(int val, int frc) : data(val), freq(frc){};
};


std::vector<Node> tree;



//can be replaced with a node struct which use indices instead of pointers so it will be 1 or 2 big arrays
std::vector<std::unique_ptr<Tree>> tree_list;

void createUnsorted(int *arr, int size, std::vector<std::unique_ptr<Tree>> &trr);

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

    createUnsorted(value_freq, 257, tree);
    merge::sort(tree_list, []( const Node &a, const Node &b ){return a.freq < b.freq;});

    


    return 0;
}

void createUnsorted(int *arr, int arr_size, std::vector<Node> &tree){
    for(int i=0; i<arr_size; i++)
    if(arr[i] >= 1) tree.push_back({i, arr[i]});
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