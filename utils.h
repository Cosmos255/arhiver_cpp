#pragma once
#include <vector>
#include <memory>
#include <utility>

constexpr int deflateBitLength = 29;

constexpr int deflateBitDist = 30;

enum type_e {lvalue, match};


struct token{
    type_e type = lvalue;
    unsigned char data;
    int len;
    int dist;
    
    token() = default;
    token(int l, int d) : len(l), dist(d), type(match) {};
    token(unsigned char d) : data(d){};
    
};

//The tree struct
struct Tree{
    int freq = 0;
    int data = '\0';

    std::unique_ptr<Tree> left;
    std::unique_ptr<Tree> right; 

    Tree() = default;

    Tree(int val, int frc) : data(val), freq(frc){}
};

struct bits{
    uint64_t bits=0;
    uint64_t length=0;
};

namespace merge{

    
    template <typename T, typename Compare>
    void merge(std::vector<T> &arr, int left, int mid, int right, const Compare method){

        int n1=mid-left+1;
        int n2=right-mid;

        std::vector<T> L(n1), R(n2);

        for(int i=0; i < n1; i++){
            L[i] = std::move(arr[left+i]);
        }
        for(int j=0; j < n2; j++){
            R[j] = std::move(arr[mid+1+j]);
        }
        int p=0, k=0;
        int pos = left;
        while(p < n1 && k <n2){
            if(method(L[p], R[k])){
                arr[pos++] = std::move(L[p++]);
            }
            else{
                arr[pos++] = std::move(R[k++]);
            }
        }
        while(p < n1){
            arr[pos++] = std::move(L[p++]);
        }
        while(k < n2){
            arr[pos++] = std::move(R[k++]);
        }
        return;

    }
    
    template <typename T, typename Compare>
    void merge_sort(std::vector<T> &arr, int left, int right, const Compare method){
        if(left>= right)
            return;


        int mid = left+(right-left)/2;
        merge_sort(arr, left, mid, method);
        merge_sort(arr, mid+1, right, method);
        merge(arr, left, mid, right, method); // need to add the lambda
    }


    template <typename T, typename Compare>
    void sort(std::vector<T> &arr, const Compare method){
        merge_sort(arr, 0, arr.size()-1, method);
        return;
    }



}

namespace binary_heap{

    struct Node{
        int value;
        int freq;

        Node(int val, int freq) : value(val), freq(freq){};
    };

    int left(int n){
        return 2*n+1;
    }
    int right(int n){
        return 2*n+2;
    }


    std::vector<Node> heap;

    int parent = 0;
    int size = 0;
    int child;

    void insert(int val, int freq){
        heap.push_back({val, freq});
        size++;
        if(size == 1) return;
        child = size-1;
        if(child%2) parent = (child-1)/2;
        else parent = (child-2)/2; 


        while(heap[parent].freq > heap[child].freq && child > 0){
            std::swap(heap[parent], heap[child]);
            child = parent;
            if(child%2) parent = (child-1)/2;
            else parent = (child-2)/2;
        }
    }

    Node extrt(){
        Node rt = std::move(heap[0]);
        size--;
        heap[0] = heap[size];
        heap.pop_back();


        int lf = 1;
        int rf = 2;
        parent = 0;
        child = (heap[lf].freq < heap[rf].freq) ? lf : rf;

        while(heap[parent].freq > heap[child].freq){
            std::swap(heap[parent], heap[child]);

            parent = child;

            lf = left(parent); //add heap limit
            rf = right(parent);
            child = (heap[lf].freq < heap[rf].freq) ? lf : rf;
        }
        return rt;
    }

}

template<typename T>
void swap(T &a, T &b){
    T buffer = std::move(a);
    a = std::move(b);
    b = std::move(buffer);
}



struct bitLenghts{
    int baselength;
    int extraBits;
};

constexpr bitLenghts lengthTable[] = {
    {3,0}, {4,0}, {5,0}, {6,0}, {7,0}, {8,0}, {9,0}, {10,0},
    {11,1}, {13,1}, {15,1}, {17,1},
    {19,2}, {23,2}, {27,2}, {31,2},
    {35,3}, {43,3}, {51,3}, {59,3},
    {67,4}, {83,4}, {99,4}, {115,4},
    {131,5}, {163,5}, {195,5}, {227,5},
    {258,0}
};
struct bitDist{
    int basedist;
    int extraBits;
};

constexpr bitDist distTable[] = {
    {1,0}, {2,0}, {3,0}, {4,0}, {5,1}, {7,1}, {9,2}, {13,2},
    {17,3}, {25,3}, {33,4}, {49,4}, {65,5}, {97,5}, {129,6}, {193,6},
    {257,7}, {385,7}, {513,8}, {769,8}, {1025,9}, {1537,9}, {2049,10}, {3073,10},
    {4097,11}, {6145,11}, {8193,12}, {12289,12}, {16385,13}, {24577,13}
};

//maybe replace with binary search

int lcode(int x){
    for(int i=0; i<deflateBitLength; i++){
        if(x < lengthTable[i].baselength){
            i--;
            return i;
        }
    }
};

int dcode(int x){
    for(int i=0; i<deflateBitDist; i++){
        if(x < distTable[i].basedist){
            i--;
            return i;
        }
    }
};


const int code_symbol = 15; //0-15 for lenghts
const int cp_previous_symbol = 16; // 3-6 times 2bits
const int repeat_zero_symbol = 17; //3-10 times 3bits
const int repeat_zero_long_symbol = 18; //11-13 7bits

const int cp_previous = 3;
const int repeat_zero = 3;
const int repeat_zero_long = 11;


/*
HEADER

BFINAL(1bit) 1 if last 0 otherwise
BTYPE(2bits)
    00 uncompresed
    01 compressed with fixed Huff
    10 comrpessed with dynamic huff
    11 reserved/error

*/


//for deflate like there are only 2 tree literals lengths and distnace 

//256 end of block

//bruh huffman codes are MSB and the rest is LSB

//last element in the tree needs to be checked lexicogrphicly so A gets 0 B 1