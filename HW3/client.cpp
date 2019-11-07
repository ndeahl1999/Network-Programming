#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <set>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "sensor.h"
#include "base_station.h"
#include <pthread.h>
#include <sstream>

using std::cin;
using std::cout;
using std::endl;
using std::string;



// helper function to detect if a sensor or base station is in range
bool in_range(int a_x, int a_y, int b_x, int b_y);


// 3 step process
// 1) if the intended recipient is in range, send to recipient
// 2) sort all in range sensors and base stations by distance to destination
//    by cartesian distance.
// 3) sender sends message to the base station with lowest distance to intended
//    recipient (that does not cause a cycle)
void send_message();



void send_update_position(char* sensor_id, int sensor_range, int x_pos, int y_pos, int sock_fd);




// can take input from std in
// move [new x pos] [new y pos]
// senddata [destination id]
// where [sensorid/baseid]
// updateposition [sensorid] [sensorrange] [current x] [current y]
// quit
void handle_input(char* sensor_id, int sock_fd);


// spawn a new thread into this function
// will keep on doing a `read` call for messages
void* listen_for(void *args);

int main(int argc, char **argv){


  char* control_address = argv[1];
  int control_port = atoi(argv[2]);
  char* sensor_id = argv[3];
  int sensor_range = atoi(argv[4]);
  int initial_x_position = atoi(argv[5]);
  int initial_y_position = atoi(argv[6]);

  Sensor s(sensor_id, sensor_range, initial_x_position, initial_y_position);

  pthread_t pid;

  int sock_fd;
  struct sockaddr_in servaddr;
  sock_fd= socket(AF_INET, SOCK_STREAM, 0);
  if(sock_fd== -1){
    printf("socket creation failed\n");
    return 0;
  }

  bzero(&servaddr, sizeof(servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(control_address);
  servaddr.sin_port = htons(control_port);

  if(connect(sock_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0){

    printf("failed to connect\n");
    return 0;

  }

  // TODO
  // send update position message
  send_update_position(sensor_id, sensor_range, initial_x_position, initial_y_position, sock_fd);

  char buffer[1025];
  int n = recv(sock_fd, buffer, 1025, 0);
  buffer[n-1] = '\0';

  // format of message
  // REACHABLE <NUM> <LIST OF REACHABLE>
  string reachable = string(buffer);
  std::istringstream iss(buffer);
  string word;
  iss >> word;

  // make sure received reachable
  if(word.compare("REACHABLE") != 0 ){
    printf("didn't receive reachable\n");
    return 0;
  }

  // get number of reachable
  int num_reachable;
  iss >> num_reachable;
  for(int i=0;i<num_reachable;i++){

    // add each to the map
    string id;
    int x;
    int y;
    iss >> id >> x >> y;
    SensorBaseStation temp(id, x, y);
    s.in_reach.insert(temp);
  }
  // DEBUG TO SEE CONTENTS OF in_reach
  // for(std::set<SensorBaseStation>::iterator it = in_reach.begin(); it != in_reach.end(); it++){
  //   cout<<it->getID() << " "<< it->getX()<<" " << it->getY()<<endl;
  // }
  

  void* status;

  // create a thread to listen for messages 
  pthread_create(&pid, NULL, listen_for, NULL);
  pthread_join(pid, &status);

  
  // main thread will start handling input  
  handle_input(sensor_id, sock_fd);

  
  
  printf("finished execution\n");
  close(sock_fd);

  return 1;

}


void handle_input(char *sensor_id,  int sock_fd){

  printf("currently handling input\n");
  string line; 
  string id(sensor_id);

  //loop and keep handling messages
  while(true){

    getline(cin, line);
    // temporary debug to make sure the server can get the message
    // send(sock_fd, line.c_str(), line.length(),0);
  
    std::istringstream iss(line);
    string word;

    while(iss >> word){
      if(word == "SENDDATA"){

        string dest_id;
        iss >> dest_id;

        // printf("%s: %s %s\n", sensor_id, word.c_str(), dest_id.c_str());
        // string message  = "DATAMESSAGE " + string(sensor_id) + " " + 



        //TODO
        // make sure this check works with string conversion
        // this shouldn't go here
        // this check should go in handle_message that is sent from control
        if(id == dest_id){
          // cout<<"matching id"<<endl;
        }

        // TODO
        // pass the message to the next on the stop list
        else{

        }
        // handle rest of send data in here
      }
      else if(word == "MOVE"){
        cout<<"we got a move data request"<<endl;
        while(iss >> word){
          cout<<"rest of data is "<< word<< endl;
        }

      }
      
      else if(word == "QUIT"){
        return;

      }
    }
  }

}

void* listen_for(void *args){




  pthread_exit(0);
  return NULL;
}


void send_update_position(char* sensor_id, int sensor_range, int x_pos, int y_pos, int sock_fd){
  string message = "UPDATEPOSITION " + string(sensor_id) + " " + std::to_string(sensor_range) + " " +  std::to_string(x_pos) +
             " " + std::to_string(y_pos) + " ";
  // cout<<message<<endl;
  int bytes = send(sock_fd, message.c_str(),  message.length(), 0);


}