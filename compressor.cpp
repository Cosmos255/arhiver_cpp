#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>

int values[256] = {0};



//Some kind of linked list maybe il use it instead of the std::vector
struct Node{
    char data;

    Node *next;
};


//The tree struct
struct Tree{
    int freq;
    char data;

    Tree *left;
    Tree *right;
};

std::vector<Tree> unsorted_tree(256); //Vector containing the branches that still need sorting

int main(int argc, char* argv[]){
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


    





    //Here should start the code which will do the
    //tree and stuff and whats above will probably be moved to 
    //read or to arhive via move being here doesnt really do a 
    //thing might even be throw into read as it will be nedeed by compressor and
    //decompresoor but ath the same time i am not sure as we need
    //the same treee so we kinda build it the same its jsust '
    //strange how exactly to go into thisgi



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





}
