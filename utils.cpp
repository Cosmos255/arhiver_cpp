#include "utils.hpp"

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

void insert(Node nd){
    heap.push_back(nd);
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

void clear(){
    heap.clear();
}


Node extrt(){
    if(size==0) return {-1 , 0};
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
        if(lf >= size) break;
        else if(rf >= size) rf=lf;
        child = (heap[lf].freq < heap[rf].freq) ? lf : rf;
    }
    return rt;
}

