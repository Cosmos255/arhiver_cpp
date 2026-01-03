#include <vector>
#include <memory>

struct Node{
    char data;

    Node *next;
};


//The tree struct
struct Tree{
    int freq = 0;
    unsigned char data = '\0';

    //Tree *left = nullptr;
    //Tree *right = nullptr;

    std::unique_ptr<Tree> left;
    std::unique_ptr<Tree> right; 

    Tree() = default;

    Tree(unsigned char val, int frc) : data(val), freq(frc){}
};

namespace merge{

    void merge(std::vector<std::unique_ptr<Tree>> &arr, int left, int mid, int right){
        int n1=mid-left+1;
        int n2=right-mid;
        std::vector<std::unique_ptr<Tree>> L(n1), R(n2);
        for(int i=0; i < n1; i++){
            L[i] = std::move(arr[left+i]);
        }
        for(int j=0; j < n2; j++){
            R[j] = std::move(arr[mid+1+j]);
        }
        int p=0, k=0;
        int pos = left;
        while(p < n1 && k <n2){
            if(L[p]->freq > R[k]->freq){
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

    void merge_sort(std::vector<std::unique_ptr<Tree>> &arr, int left, int right){
        if(left>= right)
            return;


        int mid = left+(right-left)/2;
        merge_sort(arr, left, mid);
        merge_sort(arr, mid+1, right);
        merge(arr, left, mid, right);
    }


    void sort(std::vector<std::unique_ptr<Tree>> &arr){
        merge_sort(arr, 0, arr.size()-1);
        return;
    }



}


