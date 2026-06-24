#include <iostream>
#include <fstream>
#include "../include/utils.hpp"
#include "../include/lz77.hpp"

constexpr int SEARCH_SIZE = 32*1000;
constexpr int LOOkUP_SIZE = 258;
constexpr int read_size = SEARCH_SIZE+LOOkUP_SIZE;
constexpr int min_match = 3;
constexpr int min_lookup = 10;

//need to add max limit to lenght is 258
 

std::vector<token> tokens;

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

void find_match(LZ77 &lz);
void move_window(LZ77 &lz);
void fill_window(uint64_t &read, LZ77 &lz);

std::vector<token> lz77_token(std::string file_name){
    LZ77 lz77;

    lz77.in.open(file_name , std::ios::binary |  std::ios::ate);


    if(!lz77.in.is_open()){
        throw std::runtime_error("Couldnt open the file");
    }

    uint64_t size = lz77.in.tellg();
    if(size < 1)
        throw std::runtime_error("File size too small to tokenize");
    lz77.in.seekg(0);

    lz77.ws.resize((SEARCH_SIZE+LOOkUP_SIZE), '\0');

    lz77.in.read(&lz77.ws[0], read_size);

    uint64_t read = lz77.in.gcount();
    lz77.notav = read;
    tokens.emplace_back(lz77.ws.at(lz77.srch)); //initialized the first letter


    while(lz77.look < lz77.notav){
        if(lz77.look < min_lookup+lz77.notav && !lz77.in.eof()){ 
            fill_window(read,lz77);
        }
        find_match(lz77);
        move_window(lz77);
    }
    /*
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
    */
    lz77.in.close();
    return tokens;

}


void find_match(LZ77 &lz){
    lz.match.first = 0;
    lz.match.second = 0;
    uint64_t distance = 1;

    while(lz.look >= lz.srch+distance){

        int length = 0;

        while(lz.look+length < lz.notav){ // need to modify the code so we remove
            if(lz.ws.at((lz.look+length)%read_size) == lz.ws.at((lz.look-distance+length)%read_size)) length++;
            else break;
        }
        if((length > 2) && (length > lz.match.second)){
            lz.match.first = distance;
            lz.match.second = length;
        }
        distance++;
    }

}

void move_window(LZ77 &lz){
    int length = lz.match.second; //changed second with first
    if(length == 0){
        length = 1;
        tokens.emplace_back(lz.ws.at(lz.look%read_size));
    }else{
        tokens.emplace_back(length , lz.match.first); //token(length, distance)
    }

    lz.look+=length;
    //can be replace with lz.srch = std::max(0, lz.look-SEARCH_SIZE);
    if(lz.look - lz.srch >= SEARCH_SIZE){ 
        lz.srch = lz.look-SEARCH_SIZE;
    }
}

//might remove read from lz 77 struct
void fill_window(uint64_t &read, LZ77 &lz){
    int fill_size = (lz.srch+read_size)-lz.notav; //not sure this works
    lz.in.read(&lz.ws.at(lz.notav%read_size), fill_size);
    read = lz.in.gcount();
    lz.notav+=read;
}
