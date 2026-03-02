#pragma once
#include <vector>
#include <memory>


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


