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

    std::string ws;
    uint64_t read;

    uint64_t srch = 0;
    uint64_t look = 1;
    uint64_t notav = 0;


    std::pair<int, int> match; //distance length
    //put 0 0

};

    std::vector<token> tokens;

void find_match(LZ77 &lz);
void move_window(LZ77 &lz);
void fill_window(uint64_t &read, LZ77 &lz);
uint64_t create_hash(uint64_t pos1, LZ77 &lz);
uint64_t create_hash(uint64_t hash, int pos_nxt, LZ77 &lz);

int main(){
    LZ77 lz77;

    lz77.in.open("example.txt", std::ios::binary |  std::ios::ate);


    if(!lz77.in.is_open()){
        std::cout<<"Couldnt open the file";
    }

    uint64_t size = lz77.in.tellg();
    if(size < 1) return 0;
    lz77.in.seekg(0);

    lz77.ws.resize((SEARCH_SIZE+LOOkUP_SIZE), '\0');

    lz77.in.read(&lz77.ws[0], read_size);

    uint64_t read = lz77.in.gcount();
    lz77.notav = read;
    tokens.emplace_back(lz77.ws.at(lz77.srch));


    while(lz77.look < lz77.notav){
        if(lz77.look < min_lookup+lz77.notav && !lz77.in.eof()){ 
            fill_window(read,lz77);
        }
        find_match(lz77);
        move_window(lz77);
    }

    for(token t : tokens){
        std::cout<<"TOKEN:\n";
        if(t.type == match){
            std::cout<<"match\n";
            std::cout<<t.dist<<"\t"<<t.len<<"\n";
        }else{
            std::cout<<static_cast<char>(t.data)<<"\n";
            std::cout<<"lvalue\n";
        }
    }

}




void find_match(LZ77 &lz){
    lz.match.first = 0;
    lz.match.second = 0;
    uint64_t distance = 3;

    while(lz.look >= lz.srch+distance){

        int length = 0;
        uint64_t hash = create_hash(lz.look, lz);
        uint64_t s_hash = create_hash(lz.look-distance, lz);
        length = 2; //need to do some searhing for lenght it might need to be set to 3

        while(length<distance && lz.look+length < lz.notav){ // might be without =
            if(hash == s_hash){
                length++;

            }else{
                break;
            }

            hash = create_hash(hash, lz.look+length, lz);
            s_hash = create_hash(s_hash, lz.look-distance+length, lz); // I think these two lines work who knows
        }

        if((length > 2) && (length > lz.match.second)){
            lz.match.first = distance;
            lz.match.second = length;
        }
        distance++;
    }

}



void move_window(LZ77 &lz){
    int dist = lz.match.second;
    if(lz.match.first == 0){
        dist = 1;
        tokens.emplace_back(lz.ws.at(lz.look%read_size));
    }else{
        tokens.emplace_back(lz.match.second, lz.match.first);
    }

    if(lz.look - lz.srch == SEARCH_SIZE){
        lz.srch+=dist;
        lz.look+=dist;
    }else{
        lz.look+=dist;
    }
}


//might remove read from lz 77 struct
void fill_window(uint64_t &read, LZ77 &lz){
    int fill_size = (lz.srch+read_size)-lz.notav; //not sure this works
    lz.in.read(&lz.ws.at(lz.notav%read_size), fill_size);
    read = lz.in.gcount();
    lz.notav+=read;
}

uint64_t create_hash(uint64_t pos1, LZ77 &lz){
    return (lz.ws.at(pos1%read_size) << 16) |
        (lz.ws.at((pos1+1)%read_size) << 8)|
        (lz.ws.at((pos1+2)%read_size));
}

uint64_t create_hash(uint64_t hash, int pos_nxt, LZ77 &lz){
    return ((hash & 0xFFFF) << 8) | lz.ws.at(pos_nxt%read_size);
}