#pragma once
#include <vector>
#include <memory>

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
