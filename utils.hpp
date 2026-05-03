#pragma once
#include <vector>
#include <memory>
#include <utility>




struct bits{
    uint32_t value = 0;
    uint32_t length= 0;
};

namespace bn_heap{
    struct Node{
        uint32_t value;
        int freq;
        int left = -1; 
        int right = -1;
        //left and right are for the huffman tree

        Node() = default;
        Node(int val, int freq) : value(val), freq(freq){};
    };

    void insert(Node nd);
    void clear();

    Node extrt();

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



};

struct bitLenghts{
    int baselength;
    int extraBits;
};

//remember to add 257+index for the code
//extra bits are MSB 
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
constexpr int deflateBitLength = 29;

constexpr int deflateBitDist = 30;

int lcode(int x);

int dcode(int x);


const int code_symbol = 15; //0-15 for lenghts
const int cp_previous_symbol = 16; // 3-6 times 2bits
const int repeat_zero_symbol = 17; //3-10 times 3bits
const int repeat_zero_long_symbol = 18; //11-13 7bits

const int cp_previous = 3;
const int repeat_zero = 3;
const int repeat_zero_long = 11;



const int CCL_order[] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

/*
HEADER

BFINAL(1bit) 1 if last 0 otherwise
BTYPE(2bits)
    00 uncompresed
    01 compressed with fixed Huff
    10 comrpessed with dynamic huff
    11 reserved/error


000

literal bytes 0-255
256 end of block
257-285 lengthcode

*/


//for deflate like there are only 2 tree literals lengths and distnace 

//256 end of block

//bruh huffman codes are MSB and the rest is LSB

//last element in the tree needs to be checked lexicogrphicly so A gets 0 B 1