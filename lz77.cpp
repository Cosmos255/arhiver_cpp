#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "utils.h"

const int SEARCH_SIZE = 32*1000;
const int LOOkUP_SIZE = 258;
const int read_size = SEARCH_SIZE+LOOkUP_SIZE;
const int min_match = 3;

std::vector<token> tokens;
 
//Tryna make the lz77 and see if it works or if it will work
/*
IDEA

LINKED LIST or quee for the sliding window

[SEARCH][LOOKUP]
32KB / 258bytes


ring buffer

sooo complicated -_-


*/





int main(){



    std::ifstream in;
    in.open("example.txt", std::ios::ate);
    if(!in.is_open()){
        std::cout<<"Couldnt open the file";
    }
    uint64_t size = in.tellg();
    std::string buff(size, '\0');
    in.seekg(0);
    in.getline(&buff[0], size);


    std::streamsize bytesRead = in.gcount();   

    if(bytesRead < read_size){
        
    }

    std::string window_slide((SEARCH_SIZE+LOOkUP_SIZE), '\0');

    in.read(&window_slide[0], read_size);
    unsigned long int read = in.gcount();

    uint64_t srch = 0;
    uint64_t look = 1;
    uint64_t notav = 0;

    std::pair<int, int> match; //distance length
    int distance = 1;

    while(look-distance >= srch){
        find_match();
    }



}

void find_match(uint64_t &srch, uint64_t &look, uint64_t &notav, std::string &buff){
    //int search = l_s-1 , lookup = l_s;  
    

    //need to add code for like checking how much data we have left 


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

void fill_window(){

}