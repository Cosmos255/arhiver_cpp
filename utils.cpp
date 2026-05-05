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
        if(child%2) parent = (child-1)/2;
        else parent = (child-2)/2;
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
    heap[0] = heap[size];
    heap.pop_back();
    if(size != heap.size()) throw std::runtime_error("Broken :/");

    int lf = 1;
    int rf = 2;
    parent = 0;
    child = parent;

    if(lf < size && heap[lf].freq < heap[child].freq) child=lf; //compare lf to parent.freq
    
    if(rf < size && heap[rf].freq < heap[child].freq) child=rf; //compares rf to lf.freq


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

int lcode(int x){
    for(int i=0; i<deflateBitLength; i++){
        if(x < lengthTable[i].baselength){
            i--;
            return i;
        }
    }
    return -1;
};

int dcode(int x){
    for(int i=0; i<deflateBitDist; i++){
        if(x < distTable[i].basedist){
            i--;
            return i;
        }
    }
    return -1;
};