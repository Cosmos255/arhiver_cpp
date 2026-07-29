#include <iostream>
#include <fstream>
#include "../include/utils.hpp"
#include "../include/lz77.hpp"


std::vector<token> tokens;
static bool lz_finish = true;

static struct LZ77_config{
    const int SEARCH_SIZE = 32*1000;
    const int window_size = 32*1000 + 258;
    const int min_match = 3;
    const int min_lookup = 10;
    int vec_size;
}cfg;



static struct LZ77{

    std::ifstream in;

    std::string ws;

    uint64_t srch = 0; //limits
    uint64_t look = 1; 
    uint64_t notav = 0; //limits


    std::pair<int, int> match; //distance length
    //put 0 0

}lz77;



void find_match(LZ77 &lz, const LZ77_config &cfg);
void move_window(LZ77 &lz, const LZ77_config &cfg);
void fill_window(uint64_t &read, LZ77 &lz, const LZ77_config &cfg);

std::pair<bool, std::vector<token>> lz77_token(std::string file_name, int memLevel){
    uint64_t read;
    if(lz_finish){
        lz77 = {};
        cfg.vec_size = 0;

        cfg.vec_size = (1 << (memLevel + 6)) - 1;

        tokens.clear();
        tokens.reserve(cfg.vec_size);

        lz77.in.open(file_name , std::ios::binary |  std::ios::ate);


        if(!lz77.in.is_open()) throw std::runtime_error("Couldnt open the file");
        if(lz77.in.tellg() < 1) throw std::runtime_error("File size too small to tokenize");
        
        lz77.in.seekg(0);
        lz77.ws.resize(cfg.window_size, '\0');
        lz77.in.read(&lz77.ws[0], cfg.window_size);

        read = lz77.in.gcount();
        lz77.notav = read;
        tokens.emplace_back(lz77.ws.at(lz77.srch)); //initialized the first letter
    }else{
        tokens.clear();
        read = lz77.in.gcount();
        tokens.emplace_back(lz77.ws.at(lz77.srch)); //first element to initialize
    }

    while(lz77.look < lz77.notav){
        if(lz77.look < cfg.min_lookup+lz77.notav && !lz77.in.eof()){ 
            fill_window(read,lz77,cfg);
        }
        if(tokens.size() >= cfg.vec_size){

            lz77.notav-=lz77.srch;
            lz77.look-=lz77.srch;
            lz77.srch= lz77.look;

            lz77.look++;

            lz_finish = false;
            return {lz_finish, tokens}; //lz finish should be false here
        }

        find_match(lz77,cfg);
        move_window(lz77,cfg);
    }
    lz_finish = true;

    lz77.in.close();
    return {lz_finish, tokens};

}


void find_match(LZ77 &lz, const LZ77_config &cfg){
    lz.match.first = 0;
    lz.match.second = 0;
    uint64_t distance = 1;

    while(lz.look >= lz.srch+distance){

        int length = 0;

        while(lz.look+length < lz.notav){
            if(lz.ws.at((lz.look+length)%cfg.window_size) == lz.ws.at((lz.look-distance+length)%cfg.window_size)) length++;
            else break;
        }
        if((length > 2) && (length > lz.match.second)){
            lz.match.first = distance;
            lz.match.second = length;
        }
        distance++;
    }

}

void move_window(LZ77 &lz, const LZ77_config &cfg){
    int length = lz.match.second; //changed second with first
    if(length == 0){
        length = 1;
        tokens.emplace_back(lz.ws.at(lz.look%cfg.window_size));
    }else{
        tokens.emplace_back(length , lz.match.first); //token(length, distance)
    }

    lz.look+=length;

    if(lz.look > cfg.SEARCH_SIZE) lz.srch = std::max(lz.look-cfg.SEARCH_SIZE, lz.srch);
}

//might remove read from lz 77 struct
void fill_window(uint64_t &read, LZ77 &lz, const LZ77_config &cfg){
    int fill_size = (lz.srch+cfg.window_size)-lz.notav; //not sure this works
    lz.in.read(&lz.ws.at(lz.notav%cfg.window_size), fill_size);
    read = lz.in.gcount();
    lz.notav+=read;
}
