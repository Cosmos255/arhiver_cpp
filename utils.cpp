#include "utils.hpp"

using namespace bn_heap;

int left(int n){
    return 2*n+1;
}
int right(int n){
    return 2*n+2;
}


std::vector<Node> heap;

int parent = 0;
int size = 0;
int child = 0;
 
void bn_heap::insert(Node nd){
    heap.push_back(nd);
    size++;
    if(size == 1) return;
    child = size-1;
    parent = (child-1)/2;


    while(heap[parent].freq > heap[child].freq && child > 0){
        std::swap(heap[parent], heap[child]);
        child = parent;
        parent = (child-1)/2;
    }
}

void bn_heap::clear(){
    heap.clear();
    parent=0;
    size=0;
    child =0;
}

Node bn_heap::extrt(){
    if(size==0) return {-1 , 0};
    Node rt = std::move(heap[0]);
    size--;
    heap[0] = heap[heap.size()-1];
    heap.pop_back();
    if(size != heap.size()) throw std::runtime_error("Broken :/");

    parent = 0;

    while(true){
        int lf = left(parent);
        int rf = right(parent);
        int child = parent;

        if(lf < size && heap[lf].freq < heap[child].freq) child = lf;
        if(rf < size && heap[rf].freq < heap[child].freq) child = rf;
    
        if(child == parent) break;

        std::swap(heap[parent], heap[child]);
        parent = child;
    }
    return rt;
}


int lcode(int x){
    for(int i = 0; i < deflateBitLength - 1; i++){
        if(x >= lengthTable[i].baselength && x < lengthTable[i+1].baselength) return i;
    }
    if(x >= lengthTable[deflateBitLength - 1].baselength) return deflateBitLength - 1;
    return -1;
}

int dcode(int x){
    for(int i = 0; i < deflateBitDist - 1; i++){
        if(x >= distTable[i].basedist && x < distTable[i+1].basedist) return i;
    }
    if(x >= distTable[deflateBitDist - 1].basedist) return deflateBitDist - 1;
    return -1;
}