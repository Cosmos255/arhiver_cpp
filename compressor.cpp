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


constexpr int maxCodeValue = 257;
const int MAX_BITS = 15;


int values[maxCodeValue] = {0};
//256 end of block

int bl_count[maxCodeValue];


int len[deflateBitLength] = {0};
int dist[deflateBitDist] = {0};


std::ofstream replace_this("output.arhv"); 

//Tree root;

std::vector<std::unique_ptr<Tree>> unsorted_tree; //Vector containing the branches that still need sorting
std::vector<std::unique_ptr<Tree>> unsorted_len;
std::vector<std::unique_ptr<Tree>> unsorted_dist;

int lcode(int x);
int dcode(int x);
void buildTree(std::vector<std::unique_ptr<Tree>> &tree);
void createUnsorted(int *arr, int size, std::vector<std::unique_ptr<Tree>> &trr);
void createBitcode(std::unique_ptr<Tree> &t, std::unordered_map<int, bits> &dic);
void writeBitcode(int index,std::unordered_map<int, bits> map, int &freebits, uint64_t &outBuffer);
void writeExtra(int value, int &freebits, uint64_t &outBuffer, const bool len);
void createBitcode(std::unique_ptr<Tree> &t, std::unordered_map<int, bits> &dic, bits b, const int index);
void flush(uint64_t &buffer, int &freebits);

int main(int argc, char* argv[]){

    std::unordered_map<int, bits> ctob; // key value ctob[key] == value / ctob.insert({key, value})
    std::unordered_map<int, bits> ctol; 
    std::unordered_map<int, bits> ctod;
    //bits
    //length

    std::ifstream file;
    

    unsorted_tree.reserve(257);
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


            len[lcode(tk.len)]++;
            len[dcode(tk.dist)]++;

            /*
            len[(int)(tk.len)]++;
            dist[(int)(tk.dist)]++;
            */
    }



    createUnsorted(values, 257, unsorted_tree);
    createUnsorted(len, deflateBitDist, unsorted_len);
    createUnsorted(dist, deflateBitDist, unsorted_dist);

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
    //10 compresed with dynamic huffman

    //need to output the dictiuonary and canonicals
    std::string header;
    header = 0b10; //comrpesion type
    



    for(token &t : lzed){
        if(t.type == match){

            writeBitcode(256, ctob, freebits, outBuffer);


            /*
            uint64_t disp = ctob[256].length;

            //I forgot about the length and distnace :/

            if(disp > freebits) flush(outBuffer, freebits);
            if(disp < freebits){
                outBuffer = (outBuffer<<disp) | ctob[256].bits;
                freebits-=disp;
            }
            */

            writeBitcode(t.len, ctol, freebits, outBuffer);

            writeExtra(t.len, freebits, outBuffer, 1);

            /*
            int indx = lcode(t.dist);

            int disp = lengthTable[indx].extraBits;

            int code = (ctol[t.dist].bits << disp) | (t.dist - lengthTable[indx].baselength);
            */  


            writeBitcode(t.dist, ctod, freebits, outBuffer);
            writeExtra(t.dist, freebits, outBuffer, 0);
            
            

/*
            disp = ctod[t.dist].length;
            if(disp > freebits) flush(outBuffer, freebits);
            else{
                outBuffer = (outBuffer<<disp) | ctod[t.dist].bits;
                freebits-=disp;
            }
*/

/*
            disp = ctol[t.len].length;
            if(disp > freebits) flush(outBuffer, freebits);
            else{
                outBuffer = (outBuffer<<disp) | ctol[t.len].bits;
                freebits-=disp;
            }
*/
        
        }else{

            writeBitcode(t.data, ctob, freebits, outBuffer);

            /*
            int chr = t.data;
            uint64_t disp = ctob[chr].length;
            if(disp > freebits) flush(outBuffer, freebits);
            if(disp < freebits){
                outBuffer = (outBuffer<<disp) | ctob[chr].bits;
                freebits-=disp;
            }
            */
        }
    }

    //mostly finished need to add the magiccode
    
    //10 compresed with dynamic huffman


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

void createBitcode(std::unique_ptr<Tree> &t, std::unordered_map<int, bits> &dic, bits b, const int index){
    b.bits = (b.bits<<1) | index;
    b.length++;

    if(t->left){
        createBitcode(t->left, dic, b, 0);
    }
    if(t->right){
        createBitcode(t->right, dic, b, 1);
    }



    if(!t->left && !t->right){      
        uint64_t rev=0;
        for(int i=0; i < b.length ; i++){
            rev = (rev<<1) | (b.bits & 1);
            b.bits>>=1;
        }
        dic[t->data] = b;
    }
};

void createBitcode(std::unique_ptr<Tree> &t, std::unordered_map<int, bits> &dic){
    bits b;
    if(t->left){
        createBitcode(t->left, dic, b, 0);
    }
    if(t->right){
        createBitcode(t->right, dic, b, 1);
    }

    if(!t->left && !t->right){
        uint64_t rev=0;
        for(int i=0; i < b.length ; i++){
            rev = (rev<<1) | (b.bits & 1);
            b.bits>>=1;
        }
        dic[t->data] = b;
    }
};

void writeBitcode(int index,std::unordered_map<int, bits> map, int &freebits, uint64_t &outBuffer){
    uint64_t disp;

    disp = map[index].length;
    if(disp > freebits) flush(outBuffer, freebits);
    outBuffer = (outBuffer<<disp) | map[index].bits;
    freebits-=disp;
    
}

void writeExtra(int value, int &freebits, uint64_t &outBuffer, const bool len){
    //forgot i need to add the extra bits so more code and methods for dcode abd lcode
    uint64_t disp;
    int indx;

    if(len){
        indx = lcode(value);
        disp = lengthTable[indx].extraBits;
        if(disp > freebits) flush(outBuffer, freebits);
        outBuffer = (outBuffer<<disp) | (value - lengthTable[indx].baselength);

    }else{
        indx = dcode(value);
        disp = distTable[indx].extraBits;
        if(disp > freebits) flush(outBuffer, freebits);
        outBuffer = (outBuffer<<disp) | (value - distTable[indx].basedist);
    }

    freebits-=disp;

}

void flush(uint64_t &buffer, int &freebits){

    int bytes = (64-freebits) / 8; 
    //replace_this.write(reinterpret_cast<char*>(buffer>>(8-bytes)*8), bytes); //replace with string.data so its safer and works
    replace_this.write(reinterpret_cast<char*>(&buffer), bytes); //should work also this write bottom -> up buffer 
    freebits+= bytes*8;
    //good for stirng buffer = ((buffer<<bytes*8) & 0xFFFFFFFFFFFFFFFF)>>bytes*8; //remove the top x bytes
    buffer <<=bytes*8;
    buffer >>=bytes*8;
}