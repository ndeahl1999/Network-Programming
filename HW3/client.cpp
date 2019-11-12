#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <set>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "sensor.h"
#include "base_station.h"
#include <pthread.h>
#include <sstream>
#include <strings.h>
#include <netdb.h>
#include <fcntl.h>

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
void receive_message(string message, string sensor_id, int sock_fd);


// spawn a new thread into this function
// will keep on doing a `recv` call for messages
void* listen_for(void *args);
Sensor s;

int main(int argc, char **argv){


  setvbuf( stdout, NULL, _IONBF, 0 );


  char* control_address = argv[1];
  int control_port = atoi(argv[2]);
  char* sensor_id = argv[3];
  int sensor_range = atoi(argv[4]);
  int initial_x_position = atoi(argv[5]);
  int initial_y_position = atoi(argv[6]);

  s = Sensor(sensor_id, sensor_range, initial_x_position, initial_y_position);

  pthread_t pid;

  int sock_fd;
  struct sockaddr_in servaddr;
  sock_fd= socket(AF_INET, SOCK_STREAM, 0);
  if(sock_fd== -1){
    printf("socket creation failed\n");
    return 0;
  }

  bzero(&servaddr, sizeof(servaddr));

  struct hostent *host_entry;
  host_entry = gethostbyname(control_address);

  char* host;
  host = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));


  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(host);
  servaddr.sin_port = htons(control_port);

  if(connect(sock_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0){

    printf("failed to connect\n");
    return 0;

  }

  // TODO
  // send update position message
  send_update_position(sensor_id, sensor_range, initial_x_position, initial_y_position, sock_fd);

  char buffer[1025];
  bzero(&buffer, 1025);
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

    // printf("got - %s - %d - %d\n", id.c_str(), x, y);
    SensorBaseStation temp(id, x, y);

    // add it to the sensor
    s.add_in_reach(temp);
  }
  // DEBUG TO SEE CONTENTS OF in_reach
  // for(std::set<SensorBaseStation>::iterator it = in_reach.begin(); it != in_reach.end(); it++){
  //   cout<<it->getID() << " "<< it->getX()<<" " << it->getY()<<endl;
  // }
  
  fd_set readfds;

  while (true){
    FD_ZERO(&readfds);

    FD_SET(sock_fd, &readfds);
    FD_SET(STDIN_FILENO, &readfds);

    if((select(sock_fd+1,&readfds,NULL,NULL,0)<0)){
      perror("select");
      exit(EXIT_FAILURE);
    }

    //Client has received data from a socket
    if(FD_ISSET(sock_fd, &readfds)){
      char buffer[1025];
      bzero(&buffer, 1025);
      int n = recv(sock_fd, buffer, 1025, 0);
      receive_message(string(buffer), string(sensor_id), sock_fd);
      
      //RECEIVE data and pass into function
    }

    //Client has received data from stdin
    if(FD_ISSET(STDIN_FILENO, &readfds)){

      // printf("currently handling input\n");
      string line; 
      string id(sensor_id);

      //loop and keep handling messages
      // while(true){

        getline(cin, line);
        // temporary debug to make sure the server can get the message
        // send(sock_fd, line.c_str(), line.length(),0);
      
        std::istringstream iss(line);
        string word;

        while(iss >> word){
          if(word == "SENDDATA"){
            // cout<<"we are sending"<<endl;

            string dest_id;
            iss >> dest_id;


            // use those coordinates as the comparison to get the in reach base station

            //TODO
            // make sure this check works with string conversion
            // this shouldn't go here
            // this check should go in handle_message that is sent from control
            if(id == dest_id){
              printf("%s: Message from %s to %s successfully received.\n ", sensor_id, dest_id.c_str(), sensor_id);
            }

            

            // TODO
            // pass the message to the next on the stop list
            else{
              bool found = false;
              double min_dist=1000000;
              const SensorBaseStation * closest;
              for(std::set<SensorBaseStation>::iterator it = s.in_reach.begin(); it != s.in_reach.end();it++){

                if(it->getID() == dest_id){
                  string data_message = "DATAMESSAGE " + string(s.getID()) + " " + dest_id + " "+ dest_id + " " + to_string(0) + " ";
                  printf("%s: Sent a new message bound for %s.\n", s.getID(), it->getID().c_str());
                  send(sock_fd, data_message.c_str(), data_message.length(),0);
                  
                  found = true;
                }
              }

              // if found, no need to find next person to send it to
              if(found == true){
                continue;
              }


              // couldn't find the dest_id in the list of reachable
              // now send a where to find which node to send the message to next

              // send a where 
              string message = "WHERE " + dest_id;
              send(sock_fd, message.c_str(), message.length(),0);

              // get a there back
              char buffer[1025];
              bzero(&buffer, 1025);
              int n = recv(sock_fd, buffer, 1025, 0);

              // parse the there
              std::istringstream there(buffer);
              there >> word;
              if(word != "THERE"){
                perror("sux");
                return 1;
              }

              int target_x;
              int target_y;
              there >> target_x >> target_y;

              // look through the list of base stations and see which is the closest
              for(std::set<SensorBaseStation>::iterator it = s.in_reach.begin(); it != s.in_reach.end();it++){
                  double distance = sqrt( pow( it->getX() - target_x, 2) + pow(it->getY() - target_y, 2));
                  if(distance < min_dist){
                    closest = &(*it);
                    min_dist = distance;
                  }
              }
              string data_message = "DATAMESSAGE " + string(s.getID()) + " " + closest->getID()+ " "+ dest_id + " " + to_string(0) + " ";
                  printf("%s: Sent a new message bound for %s.\n", s.getID(), dest_id.c_str());
                  send(sock_fd, data_message.c_str(), data_message.length(),0);
              }
            // handle rest of send data in here
          }

          // move locations
          else if(word == "MOVE"){
            int new_x;
            int new_y;
            iss >> new_x >> new_y;

            // set the new coordinates
            s.setX(new_x);
            s.setY(new_y);

            // send an update position command to the server
            send_update_position(sensor_id, s.getRange(), s.getX(), s.getY(), sock_fd);

            // receive new list of reachable
            char buffer[1025];
            bzero(&buffer, 1025);
            int n = recv(sock_fd, buffer, 1025, 0);

            std::istringstream iss(buffer);
            string word;
            iss >> word;
            if(word != "REACHABLE"){
              perror("sux");
              return 1;
            }

            // TODO handle more gracefully
            // reset the set of reachable base stations to the sensor
            s.in_reach = std::set<SensorBaseStation>();
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

              // add it to the sensor
              s.add_in_reach(temp);
            }


          }
          else if(word == "WHERE"){
            string message = "WHERE ";
            string dest;

            iss>>dest;
            message += dest;
            send(sock_fd, message.c_str(), message.length(),0);

            char buffer[1025];
            bzero(&buffer, 1025);
            int n = recv(sock_fd, buffer, 1025, 0);
            printf("%s\n", buffer);
          }
          
          else if(word == "QUIT"){
            exit(0);

          }
        }
      }
  }  
  
  printf("finished execution\n");
  close(sock_fd);

  return 1;

}

void receive_message(string buffer, string sensor_id, int sock_fd){
  std::istringstream iss(buffer);

  string message;
  iss >> message;
  if(message == "DATAMESSAGE"){
    string origin_id, next_id, dest_id;
    iss >> origin_id >> next_id >> dest_id;
    // printf("got %s\n", buffer.c_str());

    if(dest_id == sensor_id){
      printf("%s: Message from %s to %s succesfully received.\n", sensor_id.c_str(), origin_id.c_str(), dest_id.c_str());
      return;
    }
    else{

      printf("%s: Message from %s to %s being forwarded through %s\n", next_id.c_str(), origin_id.c_str(), dest_id.c_str(), next_id.c_str());
      int hop_length;
      std::vector<string> hop_list;
      iss>>hop_length;
      int i=0;
      while(i<hop_length){
        string hop;
        iss>>hop;
        hop_list.push_back(hop);
        i++;
      }
      hop_length+=1;
      hop_list.push_back(next_id);

      // for(int i=0;i<hop_list.size();i++){
      //   cout<<hop_list[i]<<endl;
      // }



      bool found = false;
      double min_dist=1000000;
      const SensorBaseStation * closest;


      for(std::set<SensorBaseStation>::iterator it = s.in_reach.begin(); it != s.in_reach.end();it++){

        // printf("can reach %s when supposed to go %s\n", it->getID().c_str(), dest_id.c_str());

        if(it->getID() == dest_id){
          // printf("---------- this comparison was true %s  ==  %s\n", it->getID().c_str(), dest_id.c_str());
          string data_message = "DATAMESSAGE " + origin_id+ " " + it->getID() + " "+ dest_id + " " + to_string(hop_length);
          for(int i=0;i<hop_list.size();i++){
            data_message+=" ";
            data_message+=hop_list[i];
          }

          // printf("got it -- %s\n", data_message.c_str());
          // printf("%s: Sent a new message bound for %s.\n", s.getID(), it->getID().c_str());
          send(sock_fd, data_message.c_str(), data_message.length(),0);
          
          found = true;
        }
      }

      if(found == true){
        return;
      }


      // send a where 
      string message = "WHERE " + dest_id;
      send(sock_fd, message.c_str(), message.length(),0);

      // get a there back
      char buffer[1025];
      bzero(&buffer, 1025);
      int n = recv(sock_fd, buffer, 1025, 0);

      
      // parse the there
      std::istringstream there(buffer);
      std::string word;
      there >> word;
      if(word != "THERE"){
        perror("sux");
        return;
      }

      int target_x;
      int target_y;
      there >> target_x >> target_y;


      for(std::set<SensorBaseStation>::iterator it = s.in_reach.begin(); it != s.in_reach.end();it++){
          double distance = sqrt( pow( it->getX() - target_x, 2) + pow(it->getY() - target_y, 2));
          if(distance < min_dist){
            closest = &(*it);
            min_dist = distance;
          }
      }
      string data_message = "DATAMESSAGE " + string(s.getID()) + " " + closest->getID()+ " "+ dest_id + " " + to_string(0) + " ";
          printf("%s: Sent a new message bound for %s.\n", s.getID(), dest_id.c_str());
          send(sock_fd, data_message.c_str(), data_message.length(),0);
      






      
      // hop_length+=1;
      // hop_list.push_back(next_id);
       
    }




  }
}


void handle_input(char *sensor_id,  int sock_fd){

  // printf("currently handling input\n");
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
        // cout<<"we are sending"<<endl;

        string dest_id;
        iss >> dest_id;


        // use those coordinates as the comparison to get the in reach base station

        //TODO
        // make sure this check works with string conversion
        // this shouldn't go here
        // this check should go in handle_message that is sent from control
        if(id == dest_id){
          printf("%s: Message from %s to %s successfully received.\n ", sensor_id, dest_id.c_str(), sensor_id);
        }

        

        // TODO
        // pass the message to the next on the stop list
        else{
          bool found = false;
          double min_dist=1000000;
          const SensorBaseStation * closest;
          for(std::set<SensorBaseStation>::iterator it = s.in_reach.begin(); it != s.in_reach.end();it++){

            if(it->getID() == dest_id){
              string data_message = "DATAMESSAGE " + string(s.getID()) + " " + dest_id + " "+ dest_id + " " + to_string(0) + " ";
              printf("%s: Sent a new message bound for %s.\n", s.getID(), it->getID().c_str());
              send(sock_fd, data_message.c_str(), data_message.length(),0);
              
              found = true;
            }
          }

          // if found, no need to find next person to send it to
          if(found == true){
            continue;
          }


          // couldn't find the dest_id in the list of reachable
          // now send a where to find which node to send the message to next

          // send a where 
          string message = "WHERE " + dest_id;
          send(sock_fd, message.c_str(), message.length(),0);

          // get a there back
          char buffer[1025];
          bzero(&buffer, 1025);
          int n = recv(sock_fd, buffer, 1025, 0);

          // parse the there
          std::istringstream there(buffer);
          there >> word;
          if(word != "THERE"){
            perror("sux");
            return;
          }

          int target_x;
          int target_y;
          there >> target_x >> target_y;

          // look through the list of base stations and see which is the closest
          for(std::set<SensorBaseStation>::iterator it = s.in_reach.begin(); it != s.in_reach.end();it++){
              double distance = sqrt( pow( it->getX() - target_x, 2) + pow(it->getY() - target_y, 2));
              if(distance < min_dist){
                closest = &(*it);
                min_dist = distance;
              }
          }
          string data_message = "DATAMESSAGE " + string(s.getID()) + " " + closest->getID()+ " "+ dest_id + " " + to_string(0) + " ";
              printf("%s: Sent a new message bound for %s.\n", s.getID(), dest_id.c_str());
              send(sock_fd, data_message.c_str(), data_message.length(),0);
          }
        // handle rest of send data in here
      }

      // move locations
      else if(word == "MOVE"){
        int new_x;
        int new_y;
        iss >> new_x >> new_y;

        // set the new coordinates
        s.setX(new_x);
        s.setY(new_y);

        // send an update position command to the server
        send_update_position(sensor_id, s.getRange(), s.getX(), s.getY(), sock_fd);

        // receive new list of reachable
        char buffer[1025];
        bzero(&buffer, 1025);
        int n = recv(sock_fd, buffer, 1025, 0);

        std::istringstream iss(buffer);
        string word;
        iss >> word;
        if(word != "REACHABLE"){
          perror("sux");
          return;
        }

        // TODO handle more gracefully
        // reset the set of reachable base stations to the sensor
        s.in_reach = std::set<SensorBaseStation>();
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

          // add it to the sensor
          s.add_in_reach(temp);
        }


      }
      else if(word == "WHERE"){
        string message = "WHERE ";
        string dest;

        iss>>dest;
        message += dest;
        send(sock_fd, message.c_str(), message.length(),0);

        char buffer[1025];
        bzero(&buffer, 1025);
        int n = recv(sock_fd, buffer, 1025, 0);
        printf("%s\n", buffer);
      }
      
      else if(word == "QUIT"){
        exit(0);

      }
    }
  }

}

void* listen_for(void *args){


  int sock_fd = (*(int *)args);
  // char buffer[1025];

  //   if(fcntl(sock_fd, F_SETFL, fcntl(sock_fd, F_GETFL) | O_NONBLOCK) < 0){
  //     printf("couldnt' set\n");
    // }
  // while(true){

// while(true){


//     bzero(&buffer, 1025); 
//     int n = recv(sock_fd, buffer, 1025, 0);
//     if(errno == EAGAIN || errno == EWOULDBLOCK){
//     }
//     // else{
//       printf("gottem\n");
    // }

    // while((n = recv(sock_fd, buffer, 1025, 0)) < 1 ){
    //   if(errno == EAGAIN || errno == EWOULDBLOCK){
    //     printf("this went through\n");
    //   }
    // }
    //     printf("other statement\n");
    // printf("received %s\n", buffer);
        

        
  // }
// }


// }
  pthread_exit(0);
  return NULL;
}


void send_update_position(char* sensor_id, int sensor_range, int x_pos, int y_pos, int sock_fd){
  string message = "UPDATEPOSITION " + string(sensor_id) + " " + std::to_string(sensor_range) + " " +  std::to_string(x_pos) +
             " " + std::to_string(y_pos) + " ";
  // cout<<message<<endl;
  int bytes = send(sock_fd, message.c_str(),  message.length(), 0);


}
