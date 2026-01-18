#include <iostream>
#include <string>
#include <fstream>
#include <vector>

const int SEARCH_SIZE = 32*1000;
const int LOOkUP_SIZE = 258;
const int read_size = SEARCH_SIZE+LOOkUP_SIZE;

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

    in.getline(&window_slide[0], SEARCH_SIZE+LOOkUP_SIZE);

    char *search = &window_slide[0];
    char *lookahead = &window_slide[1]; // not sure if its the best maybe il switch to  a vector



}