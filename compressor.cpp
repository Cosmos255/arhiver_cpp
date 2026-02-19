#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>
#include "utils.h"
#include "lz77.h"



int values[257] = {0};

//Tree root;

std::vector<std::unique_ptr<Tree>> unsorted_tree; //Vector containing the branches that still need sorting
std::vector<std::unique_ptr<Tree>> unsorted_len;
std::vector<std::unique_ptr<Tree>> unsorted_dist;

void buildTree(std::vector<std::unique_ptr<Tree>> &tree);

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

    /*
    

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
        values[(int)(element)]++;  //yea that is broke or maybe not who knows
    }

    //Create a vector of all the elements and idk do tree stuff


    for(int i = 0; i < 256; i++){
        if(values[i] >= 1){
            unsorted_tree.push_back(std::make_unique<Tree>(static_cast<unsigned char>(i), values[i]));
            std::cout<<"\n"<<unsorted_tree.back()->data<<"\t"<<unsorted_tree.back()->freq<<"\n";
        }
    }
*/

   // auto lzed = lz77_token("file.example");
  //  for(token &tk : lzed){
  //      if(tk.type == match)
 //           values[256]++;
  //      else
         //   values[(int)(tk.data)]++;
  //  }



    //sort the array with merge sort doesnt return anything

    std::vector<int> arr = {1, 2, 3 , 50 , 0, 5, 69, -5 };


    merge::sort(arr, [](int a, int b){return (a < b); }); //it doesnt return anything

    std::cout<<"\n";
    for(auto &x : arr){
        std::cout<<x<<"\n";
    }

    //merge::sort(unsorted_tree, []( const std::unique_ptr<Tree> &a, const std::unique_ptr<Tree> &b ){return a->freq > b->freq;}); //it doesnt return anything

    /*
    for(Tree *Tree : unsorted_tree){
        std::cout<<arr.freq<<"\t";
    }
    */
    //building the treee

    buildTree(unsorted_tree);
    auto root = std::move(unsorted_tree.at(0));
    unsorted_tree.clear();

    std::cout<<"\n"<<root->left->data;



    return 0;

}

void buildTree(std::vector<std::unique_ptr<Tree>> &tree){
    while(tree.size() > 1){
        auto right = std::move(tree.back());
        tree.pop_back();
        auto left = std::move(tree.back());
        tree.pop_back();

        auto parent = std::make_unique<Tree>();
        parent->freq = right->freq + left->freq;
        parent->left = std::move(left);
        parent->right = std::move(right);
        tree.push_back(std::move(parent));
    }
}