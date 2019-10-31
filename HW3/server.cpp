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
#include "base_station.h"

using std::cout;
using std::endl;
using std::string;


unsigned short control_port;
std::list<BaseStation> base_stations;

int main(int argc, char ** argv){
  if(argc != 3){
    fprintf(stderr, "ERROR: Invalid number of arguments given (expected 2).\n"); 
    exit(EXIT_FAILURE);
  }

  control_port = atoi(argv[1]);
  char* bs_file = argv[2];

  std::ifstream bs_list(bs_file);
  std::string line; 

  // while there's more base stations to read
  while(getline(bs_list, line)){
    std::istringstream ss(line);

    cout<<line<<endl;

    string temp;
    string base_id;
    int xpos;
    int ypos;
    int num_links;
    std::list<string> links_list;
    ss >> base_id;
    ss >> xpos;
    ss >> ypos;
    ss >> num_links;
    int i = 0;
    while(i < num_links ){
      ss >> temp;
      links_list.push_back(temp);
      i++;
    } 

    BaseStation new_base_station(base_id, xpos, ypos, links_list);

    base_stations.push_back(new_base_station);

    cout << "Created new base station " + base_id << " with xpos " << xpos << " and ypos " << ypos <<" with " << num_links << "links\n"; 
  }


  //create a server
  struct sockaddr_in address;
  int opt = 1;
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_fd == 0){
    perror("Socket failed");
    exit(EXIT_FAILURE);
  }

  /*
  if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  */
  address.sin_family= AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(control_port);

  if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){

    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  if(listen(server_fd, 10)<0){
    perror("listen");
    exit(EXIT_FAILURE);
  }


  while(true){

    getline(cin, line);
    cout<<line<<endl;


  }

}
