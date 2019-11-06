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
#include <pthread.h>
#include <map>
#include "base_station.h"
#include "sensor.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;


// holds list of all base stations attached
std::list<BaseStation> base_stations;
map<string, Sensor> sensors;

void * handle_sensors(void * arg){
  int server_fd = (*(int*) arg);
  char buffer[1025];
  while(true){
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);
    int conn_fd = accept(server_fd, (struct sockaddr *)&cli, &len);

    int n = recv(conn_fd, buffer, sizeof(buffer), 0);

    string sensor_msg(buffer);
    std::istringstream ss(sensor_msg);
    string msg;
    ss >> msg;

    if(msg.compare("UPDATEPOSITION")== 0){
      string reachable = "REACHABLE\n";
      char* ID;
      int range, x_pos, y_pos;
      ss >> ID; 
      ss >> range;
      ss >> x_pos;
      ss >> y_pos;


      Sensor new_sensor(ID, range, x_pos, y_pos, conn_fd);
      sensors.insert(pair<string, Sensor>(string(ID), new_sensor));

      send(conn_fd, reachable.c_str(), reachable.length(),0);
      printf("RECEIVED UPDATE POSITION\n");
    }else{

    }

    

  }
} 


int main(int argc, char ** argv){
  if(argc != 3){
    fprintf(stderr, "ERROR: Invalid number of arguments given (expected 2).\n"); 
    exit(EXIT_FAILURE);
  }

  unsigned short control_port = atoi(argv[1]);
  char* bs_file = argv[2];

  std::ifstream bs_list(bs_file);
  std::string line; 

  // while there's more base stations to read
  while(getline(bs_list, line)){

    // streamify a single line
    std::istringstream ss(line);

    cout<<line<<endl;

    // read in contents of a line into proper variables
    string temp, base_id;
    int x_pos, y_pos, num_links;
    std::list<string> links_list;
    ss >> base_id;
    ss >> x_pos;
    ss >> y_pos;
    ss >> num_links;
    int i = 0;
    while(i < num_links ){
      ss >> temp;
      links_list.push_back(temp);
      i++;
    } 

    BaseStation new_base_station(base_id, x_pos, y_pos, links_list);

    base_stations.push_back(new_base_station);

    cout << "Created new base station " + base_id << " with xpos " << x_pos << " and ypos " << y_pos<<" with " << num_links << " link(s)\n"; 
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


  //create thread to handle incoming connections from sensors
  pthread_t tid;
  pthread_create(&tid, NULL, handle_sensors, &server_fd);


  //Wait for input from stdin
  while(true){
    getline(cin, line);
  }
  

}
