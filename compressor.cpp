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



int values[257] = {0};

int len[LOOkUP_SIZE] = {0};
int dist[SEARCH_SIZE+1] = {0};


std::ofstream replace_this;
//Tree root;

std::vector<std::unique_ptr<Tree>> unsorted_tree; //Vector containing the branches that still need sorting
std::vector<std::unique_ptr<Tree>> unsorted_len;
std::vector<std::unique_ptr<Tree>> unsorted_dist;

void buildTree(std::vector<std::unique_ptr<Tree>> &tree);
void createUnsorted(int *arr, int size, std::vector<std::unique_ptr<Tree>> &trr);
void createBitcode(std::unique_ptr<Tree> &t, std::unordered_map<int, bits> &ctob);
void createBitcode(std::unique_ptr<Tree> &t, std::unordered_map<int, bits> &ctob, bits b, const int index);

int main(int argc, char* argv[]){

    std::unordered_map<int, bits> ctob; // key value ctob[key] == value / ctob.insert({key, value})
    std::unordered_map<int, bits> ctol; 
    std::unordered_map<int, bits> ctod;
    //bits
    //length

    std::ifstream file;
    

    unsorted_tree.reserve(256);
    //std::fstream file("example.txt", std::ios::binary | std::ios::in); //std::ios::binary, std::ios::in

    if(argc==1){
        //Print smth like hello and usage
        //Maybe even create a menue for smth like selecting
        //but probably not
    }else if(argc==2){
        //Selected output file no specific name
    }else {
        //Added remote name for output file
    }

    /*
    

    file.open("example.txt", std::ios::binary | std::ios::ate); //Open the file in binary mode and puts the pointer to the end of file with ate
    if(!file.is_open()) throw std::runtime_error("Couldnt open the target file"); //Checks if the file opened
    uint64_t size = file.tellg(); //Reads the size of the file
    std::string buff(size, '\0'); //Creates the buffer
    file.seekg(0); //Goes to pos 0
    if(file.read(&buff[0], size)){
        std::cout<<buff<<"\n";        
    } //Reads and outputs the file

    //Get the frequency of the chars in the buffer inside the ascii vallue array
    for (char &element : buff){
        values[(int)(element)]++;  //yea that is broke or maybe not who knows
    }

    //Create a vector of all the elements and idk do tree stuff


    for(int i = 0; i < 256; i++){
        if(values[i] >= 1){
            unsorted_tree.push_back(std::make_unique<Tree>(static_cast<unsigned char>(i), values[i]));
            std::cout<<"\n"<<unsorted_tree.back()->data<<"\t"<<unsorted_tree.back()->freq<<"\n";
        }
    }
*/

    auto lzed = lz77_token("file.example");
        for(token &tk : lzed){
            if(tk.type == match) values[256]++;
            else values[(int)(tk.data)]++;
            len[(int)(tk.len)]++;
            dist[(int)(tk.dist)]++;
    }

    createUnsorted(values, 257, unsorted_tree);
    createUnsorted(len, LOOkUP_SIZE, unsorted_len);
    createUnsorted(dist, SEARCH_SIZE+1, unsorted_dist);

    merge::sort(unsorted_tree, []( const std::unique_ptr<Tree> &a, const std::unique_ptr<Tree> &b ){return a->freq < b->freq;});
    merge::sort(unsorted_len, []( const std::unique_ptr<Tree> &a, const std::unique_ptr<Tree> &b ){return a->freq < b->freq;});
    merge::sort(unsorted_dist, []( const std::unique_ptr<Tree> &a, const std::unique_ptr<Tree> &b ){return a->freq < b->freq;});


    buildTree(unsorted_tree);
    buildTree(unsorted_len);
    buildTree(unsorted_dist);

    auto rootHuff = std::move(unsorted_tree.at(0));
    auto rootLen = std::move(unsorted_len.at(0));
    auto rootDist = std::move(unsorted_dist.at(0));
    unsorted_tree.clear();
    unsorted_len.clear();
    unsorted_dist.clear();

    //outputing the raw data :3

    createBitcode(rootHuff, ctob);
    createBitcode(rootLen, ctol);
    createBitcode(rootDist, ctod);



    uint64_t outBuffer = 0;
    int freebits=64;

    for(token &t : lzed){
        if(t.type == match){
            uint64_t disp = ctob[256].length;
            //I forgot about the length and distnace :/
            if(disp > freebits) flush(outBuffer, freebits);
            if(disp < freebits){
                outBuffer = (outBuffer<<disp) | ctob[256].bits;
                freebits-=disp;
            }
        
        }else{
            int chr = t.data;
            uint64_t disp = ctob[chr].length;
            if(disp > freebits) flush(outBuffer, freebits);
            if(disp < freebits){
                outBuffer = (outBuffer<<disp) | ctob[chr].bits;
                freebits-=disp;
            }
        }
    }
    
   


    return 0;

}   

//left 0 right 1

void createUnsorted(int *arr, int size, std::vector<std::unique_ptr<Tree>> &trr){
    for(int i=0; i<size; i++){
        if(arr[i] >= 1) trr.push_back(std::make_unique<Tree>(i, values[i]));
    }
}

void buildTree(std::vector<std::unique_ptr<Tree>> &tree){

    while(tree.size() > 1){
        auto right = std::move(tree.back());
        tree.pop_back();
        auto left = std::move(tree.back());
        tree.pop_back();

        auto parent = std::make_unique<Tree>();
        parent->freq = right->freq + left->freq;
        parent->left = std::move(left);
        parent->right = std::move(right);
        tree.push_back(std::move(parent));
    }
}

void createBitcode(std::unique_ptr<Tree> &t, std::unordered_map<int, bits> &ctob, bits b, const int index){
    b.bits = (b.bits<<1) | index;
    b.length++;

    if(t->left){
        createBitcode(t->left, ctob, b, 0);
    }
    if(t->right){
        createBitcode(t->right, ctob, b, 1);
    }

    if(!t->left && !t->right){
        uint64_t rev=0;
        for(int i=0; i < b.length ; i++){
            rev = (rev<<1) | (b.bits & 1);
            b.bits>>=1;
        }
        ctob[t->data] = b;
    }
};

void createBitcode(std::unique_ptr<Tree> &t, std::unordered_map<int, bits> &ctob){
    bits b;
    if(t->left){
        createBitcode(t->left, ctob, b, 0);
    }
    if(t->right){
        createBitcode(t->right, ctob, b, 1);
    }

    if(!t->left && !t->right){
        uint64_t rev=0;
        for(int i=0; i < b.length ; i++){
            rev = (rev<<1) | (b.bits & 1);
            b.bits>>=1;
        }
        ctob[t->data] = b;
    }
};

void flush(uint64_t &buffer, int &freebits){
    int bytes = (64-freebits) / 8;
    replace_this.write((buffer>>(3-bytes)), bytes);
    freebits+= bytes*8;
    
}