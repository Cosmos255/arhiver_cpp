#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>

int values[256] = {0};

std::ifstream file; //std::ios::binary, std::ios::in

int main(int argc, char* argv[]){

    if(argc==1){
        //Print smth like hello and usage
        //Maybe even create a menue for smth like selecting
        //but probably not
    }else if(argc==2){
        //Selected output file no specific name
    }else {
        //Added remote name for output file
    }

    file.open("example.txt", std::ios::binary, std::ios::in); //Opens the file in binary mode

    auto size = file.tellg(); //Reads the size of the file
    std::string buff(size, '\0'); //Creates the buffer
    file.seekg(0); //Goes to pos 0
    if(file.read(&buff[0], size))
        std::cout << buff <<"\n"; //Prints out the buffer which we read



    //Here should start the code which will do the
    //tree and stuff and whats above will probably be moved to 
    //read or to arhive via move being here doesnt really do a 
    //thing might even be throw into read as it will be nedeed by compressor and
    //decompresoor but ath the same time i am not sure as we need
    //the same treee so we kinda build it the same its jsust '
    //strange how exactly to go into this





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







}
