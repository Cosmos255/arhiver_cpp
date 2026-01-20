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

    in.getline(&window_slide[0], read_size);

    uint64_t srch = 0;
    uint64_t look = 1;

    while(srch-look == 0);


}

void find_match(int &srch, int &look, std::string &buff){
    //int search = l_s-1 , lookup = l_s;  
    std::vector<std::pair<int , int>> matches; //length distance


    while(search>=p_s){
        int length=1;
        lookup = l_s;
        int distance = lookup-search;

        if(buff.at(search%read_size)==buff.at(l_s%read_size)){
            while( search+1 < l_s && (search%read_size)+1 != p_s && buff.at((search%read_size)+1)==buff.at(lookup%read_size)){
                length++;
                search++;
                lookup++;
            }
            if(length>3){
                matches.emplace_back(length, distance);
            }else{
                continue;
            }
        }


        while(buff.at((search%read_size))==buff.at(lookup%read_size))


        search=search-distance;
    }

    int distance = 1;
    //srch and look both represent the pointer to the start of lookup and search
    while(look-distance >= srch){
        int length = 0;

        while(buff.at(look-distance+length)==buff.at(look+length)){
            length++;
        }
    }



};




void move_window(int dist){

}

void fill_window(){

}