#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <list>
#include <sstream>
#include "Bstation.h"
using namespace std;

unsigned short control_port;
list<Bstation> b_stations;

int main(int argc, char ** argv){
  if(argc != 3){
    fprintf(stderr, "ERROR: Invalid number of arguments given (expected 2).\n"); 
    exit(EXIT_FAILURE);
  }

  control_port = atoi(argv[1]);
  char* bs_file = argv[2];

  ifstream bs_list(bs_file);
  string line; 

  while(bs_list >> line ){
    istringstream ss(line);

    string temp;
    string base_id;
    int xpos;
    int ypos;
    int num_links;
    list<string> links_list;
    ss >> base_id;
    ss >> temp;
    xpos = stoi(temp);
    ss >> temp;
    ypos = stoi(temp);
    ss >> temp;
    num_links = stoi(temp);
    int i = 0;
    while(i < num_links ){
      ss >> temp;
      links_list.push_back(temp);
      i++;
    } 

    Bstation newbs(base_id, xpos, ypos, links_list);

    b_stations.push_back(newbs);

    cout << "Created new base station " + base_id << " with xpos " << xpos << " and ypos " << ypos <<" with " << num_links << "links\n"; 
  }




}