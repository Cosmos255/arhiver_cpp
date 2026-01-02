#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <memory>
#include "utils.h"


int values[256] = {0};

//Tree root;

std::vector<std::unique_ptr<Tree>> unsorted_tree; //Vector containing the branches that still need sorting

int main(int argc, char* argv[]){
    unsorted_tree.reserve(256);
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
    std::ifstream file;
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
        values[int(element)]++;
    }

    //Create a vector of all the elements and idk do tree stuff

    for(int &freq : values){
        if(freq >= 1){
            unsorted_tree.push_back(std::make_unique<Tree>((char)freq, freq));
        }
    }
    //sort the array with merge sort doesnt return anything
    merge::sort(unsorted_tree); //it doesnt return anything

    /*
    for(Tree *Tree : unsorted_tree){
        std::cout<<arr.freq<<"\t";
    }
    */
    //building the treee



    //Here should start the code which will do the
    //tree and stuff and whats above will probably be moved to 
    //read or to arhive via move being here doesnt really do a 
    //thing might even be throw into read as it will be nedeed by compressor and
    //decompresoor but ath the same time i am not sure as we need
    //the same treee so we kinda build it the same its jsust '
    //strange how exactly to go into thisgi


    //Tree *root;

   // root = buildTree();


   /*

    //Might switch to if stream if i understand how it works
    FILE *fileptr;
    char *buffer;
    long filelen;

    fileptr = fopen("myfile.txt", "rb");  // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    filelen = ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);                      // Jump back to the beginning of the file

    buffer = (char *)malloc(filelen * sizeof(char)); // Enough memory for the file
    fread(buffer, 1, filelen, fileptr); // Read in the entire file
    fclose(fileptr); // Close the file

*/
    return 0;

}

Tree buildTree(std::vector<std::unique_ptr<Tree>> &tree){
    if(tree.size() > 1){
        size_t pos_l = tree.size()-2;
        size_t pos_r = tree.size()-1;

        if(tree.at(pos_l).freq < tree.at(pos_r).freq){
            std::swap(pos_l, pos_r);
        }
        Tree node;
        node.freq = tree[pos_l].freq+tree[pos_r].freq;
        Tree left = std::move(tree[pos_l]);
        Tree right = std::move(tree[pos_r]);
        node.left = std::move(left);
        node.right = std::move(right);


    }else{

    }
}