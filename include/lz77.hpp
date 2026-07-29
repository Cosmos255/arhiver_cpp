#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "utils.hpp"

enum type_e {lvalue, match};


struct token{
    type_e type = lvalue;
    unsigned char data;
    int len;
    int dist;
    
    token() = default;
    token(int l, int d) : len(l), dist(d), type(match) {};
    token(unsigned char d) : data(d){};
    
};

std::pair<bool, std::vector<token>> lz77_token(std::string file_name, int memLevel = 8);



