//  extern "C"{
//    #include "unp.h"
//  }
#include <iostream>
#include <algorithm>
using namespace std;


int seed;
int port;
string dic_file;
int long_word_len;

int main(int argc, char ** argv){
  if(argc < 4){
    cerr << "Error: Invalid Number of Arguments\n";
    exit(EXIT_FAILURE);
  }

  seed = atoi(argv[1]);
  port = atoi(argv[2]);
  dic_file = argv[3];
  long_word_len = atoi(argv[4]);

  //cout << "Seed is " << seed << " on port " << port << "\n";

  

}