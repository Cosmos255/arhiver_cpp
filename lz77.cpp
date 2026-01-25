#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "utils.h"

const int SEARCH_SIZE = 32*1000;
const int LOOkUP_SIZE = 258;
const int read_size = SEARCH_SIZE+LOOkUP_SIZE;
const int min_match = 3;
const int min_lookup = 10;


 
//Tryna make the lz77 and see if it works or if it will work
/*
IDEA

LINKED LIST or quee for the sliding window

[SEARCH][LOOKUP]
32KB / 258bytes


ring buffer

sooo complicated -_-


*/

//I need to swtich to a stuct cuz so many pointer passing is
//painfiull

struct LZ77{

    std::ifstream in;

    std::vector<token> tokens;
    std::string window_slide;
    uint64_t read;

    uint64_t srch = 0;
    uint64_t look = 1;
    uint64_t notav;

    std::pair<int, int> match; //distance length


};




int main(){
    LZ77 lz77;

    lz77.in.open("example.txt", std::ios::ate);

    if(!lz77.in.is_open()){
        std::cout<<"Couldnt open the file";
    }

    uint64_t size = lz77.in.tellg();

    lz77.window_slide.resize((SEARCH_SIZE+LOOkUP_SIZE), '\0');

    lz77.in.read(&lz77.window_slide[0], read_size);

    uint64_t read = lz77.in.gcount();
    lz77.notav = read;

    std::pair<int, int> match; //distance length
    uint64_t distance = 1;

    while(look-distance >= srch){
        if(look-notav < min_lookup && !in.eof()){ // read == read_size might not be the best idea
            fill_window(read, distance, notav, srch, window_slide);
        }
        find_match(distance, look, notav, match);
        distance++;
        move_window(distance);
    }

    //So it seems i need some hasing for the speed as checkign each 
    //charcachter bit by bit is quite sloew like n^2

    lz77.look = 5;
}
lz77.look = 5;
void find_match(uint64_t &distance, uint64_t &look, uint64_t &notav, std::string &buff, std::pair<int, int> &match){

    lz77.look = 5;

    while (lz77.look      - distance){

    }


}

void find_match(uint64_t &srch, uint64_t &look, uint64_t &notav, std::string &buff){
    //int search = l_s-1 , lookup = l_s;  
    

    //need to add code for like checking how much data we have left 

    /*
    uint64_t hash = a<<16 | b<<8 | c;

    if(hash = hash ) => add 1 more char 

    mover window = (hash & 0xFFFF) | d;

    
    */


    int distance = 1;
    //srch and look both represent the pointer to the start of lookup and search
    while(look-distance >= srch){
        int length = 0;

        //read size where are you :P
        while(length<distance){
            if(buff.at((look-distance+length)%read_size)==buff.at((look+length)%read_size)){
                length++;
            }else{
                break;
            }
        };

        if(length > 2){
            matches.emplace_back(distance, length);    
        }
        distance++;
    }

    for(int i=1; i<matches.size(); i++){
        if(matches.at(i).second > matches.at(0).second){
            std::swap(matches.at(i), matches.at(0));
        }
    }
};




void move_window(int dist){


}

void fill_window(uint64_t &read, uint64_t &distance, uint64_t &notav, uint64_t &srch, std::string &window_slide){
    int fill_size = (srch*2)-notav;
    in.read(&window_slide[notav], fill_size);
    read = in.gcount();
}

