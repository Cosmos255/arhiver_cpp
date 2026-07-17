#include "include/arhv.hpp"
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>

//using namespace std;
using namespace std::filesystem;

bool outflag = 0;
path outpath = "";
bool cmpr = 0;
bool extr = 0;
c_type mode = DYNAMIC;

void programInit(path file_path, path out_path);
void checkDirectory(path dir_path, path out_path);

int main(int argc, char *argv[]){

    std::vector<path> input;

    if(argc < 2){
        std::cerr<<"Not enough arguments check usage --help";
        return 0;
    }

    for(int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if(arg == "-h" || arg == "--help") {
            std::cout << "Usage:\n '-x' :decompress a raw deflate file \n '-c' :compress a file into raw deflate \n '-o' specify an output folder";
            std::cout << "\nMethod:\n '--mode=static  : faster compression with lower signature header but less efficient compared to dynamic";
            std::cout <<"\n '--mode=dynamic'  : uses dynamic created codes based on the data it creates the highest compression ratio";
            std::cout <<"\n '--mode=stored'  : used to specify a stored block";
            std::cout <<"\nThe default mode is dynamic\n\n";

            return 0;
        }
        else if(arg == "-x") {
            if(cmpr) { std::cerr << "Cannot use -x with -c\n"; return 1;}
            extr = 1;
        }
        else if(arg == "-c") {
            if(extr) { std::cerr << "Cannot use -c with -x\n"; return 1;}
            cmpr = 1;
        }
        else if(arg == "-o") outflag = 1;
        else if(arg == "--mode=static") mode = STATIC;
        else if(arg == "--mode=dynamic") mode = DYNAMIC;
        else if(arg == "--mode=stored") mode = STORED;
        else {
            if(outflag) {
                outpath = argv[i];
                outflag = 0;
            }
            else input.push_back(argv[i]);
        }
    }

    if(!extr && !cmpr){
        std::cerr<<"\nNo operation flag was specified";
        return 0;
    }


    if(input.empty()){ std::cerr<<"\nNo input file provided"; return 0; }
    if(outpath.empty()) outpath = (cmpr) ? "arhived" : "dearhived";

    if(!exists(outpath)) create_directory(outpath);


    for(path &entry : input){
        if(!exists(entry) || is_empty(entry)){ 
            std::cerr<<"\nNotice: '"<<entry<<"' does not exist or is empty";
            //mayeb add do you want to continue
            continue;
        }
        if(is_directory(entry)) checkDirectory(entry, outpath);
        else programInit(entry, outpath);
    }

    path mypth;
    mypth += ".txt";

    std::cout<<"\nFinished operations";
    return 0;
}

void programInit(const path file_path, path out_path){
    out_path /= file_path.stem();
    out_path += ".bin";
    if(extr) decompress(file_path.string(), out_path.string());
    else compress(file_path.string(), out_path.string(), mode);
}


void checkDirectory(path dir_path, path out_path){
    out_path/= dir_path.filename();//create a directory with the same name
    create_directory(out_path); 
    for(const auto &entry : directory_iterator(dir_path)){
        if(is_empty(entry.path())) continue;
        if(is_directory(entry.path())) checkDirectory(entry.path(), out_path);
        else programInit(entry.path(), out_path);
    }
}

