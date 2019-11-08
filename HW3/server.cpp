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
#include <cmath>
#include "base_station.h"
#include "sensor.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;


bool in_range(int station_x, int station_y, int sensor_x, int sensor_y, int max_dist){

  double distance = sqrt( pow(station_x - sensor_x, 2) + pow(station_y - sensor_y, 2));
  if(distance > max_dist){
    return false;
  }
  // use distance formula  here

  // return whether or not control can reach sensor
  return true;
}



// holds list of all base stations attached
// std::set<BaseStation> base_stations;
// std::set<string> base_station_names;
map<string, BaseStation> base_stations;
map<string, Sensor> sensors;
int server_fd;

string find_next_hop(string base_id, string dest_id, std::list<string>hop_list){
  BaseStation *curr_b = &(base_stations[base_id]);
  BaseStation *dest_b = &(base_stations[dest_id]);

  set<string> dest_list  = dest_b->getLinksList();
  if(dest_list.find(base_id) != dest_list.end()){
    return dest_id;
  }


  return "Nothing";
}

string get_reachable_message(int a_x, int a_y, int range){
  string reachable = "REACHABLE ";
  // append all the base stations to reachable
  int counter=0;
  string reachable_list = "";
  for(std::pair<string, BaseStation> entry : base_stations){
    BaseStation *b = &(entry.second);

    if(in_range(b->getX(), b->getY(), a_x, a_y, range)){
      // its in range
      // printf("this base station is in range\n");
      // printf("%s %d %d\n", b.getID().c_str(), b.getX(), b.getY());
      reachable_list+=b->getID();
      reachable_list+= " ";
      reachable_list+=to_string(b->getX());
      reachable_list+= " ";
      reachable_list+=to_string(b->getY());
      reachable_list+= " ";
      counter++;
    }
  }
  reachable+=to_string(counter);
  reachable+=" ";
  reachable+=reachable_list;


  return reachable;


}
// this  talks to a specific sensor
void * handle_single_sensor(void* arg){
  char*ID = (char *) arg;
  // printf("Created new thread for sensor %s\n", ID);

  Sensor s = sensors.find(string(ID))->second;
  int conn_fd = s.getFD();
  string reachable = get_reachable_message(s.getX(), s.getY(), s.getRange());


  send(conn_fd, reachable.c_str(), reachable.length(),0);

  int n;
  char buffer[1025];
  
  while(true){
    bzero(&buffer, 1025);
    n = recv(conn_fd, buffer, 1025, 0);
    string message = string(buffer);
    std::istringstream iss(message);
    string word;
    iss >> word;
    if(word == "DATAMESSAGE"){
      string origin_id;
      string next_id;
      string dest_id;

      // TODO
      // handle these
      int hop_length;
      std::list<string> hop_list;

      iss >> origin_id >> next_id >> dest_id;
      //if the message has reached its destintation
      if(next_id == dest_id){
        //message has reached is destination succesfully
        printf("%s: Message from %s to %s successfully received.\n", dest_id.c_str(), origin_id.c_str(), next_id.c_str());  
      }else{
        iss>>hop_length;
        int i = 0;
        //read string for each hop already done. 
        while(i < hop_length){
          string hop;
          iss >> hop;
          hop_list.push_back(hop);
          i++;
        }
        hop_length +=1;
        hop_list.push_back(next_id);
        //add string for hop to current base station 
        printf("%s: Message from %s to %s being forwarded through %s\n", next_id.c_str(), origin_id.c_str(), dest_id.c_str(), next_id.c_str());

        string next_hop = find_next_hop(next_id, dest_id, hop_list);
        string new_dm = "DATAMESSAGE " + origin_id + " " + next_hop + " " + to_string(hop_list.size()); 
        for(string base_name : hop_list){
          new_dm += " " + base_name;
        }
        new_dm += "\n";
        send(server_fd, new_dm.c_str(), new_dm.length(),0);

      }
    }
    else if(word == "WHERE"){
      string target;
      iss >> target;
      if(base_stations.find(target) != base_stations.end()){
        // printf("got it from base statopms\n");
        for(std::map<string, BaseStation>::iterator it = base_stations.begin();it != base_stations.end(); it++ ){
          if(it->second.getID() == target){
            string message = "THERE ";
            message += to_string(it->second.getX());
            message += " ";
            message += to_string(it->second.getY());
            send(conn_fd, message.c_str(), message.length(), 0);

          }
        }
      }
      else if(sensors.find(target) != sensors.end()){
        // printf("got it from sensors\n");
        string message = "THERE ";
        message += to_string(sensors[target].getX());
        message += " ";
        message += to_string(sensors[target].getY());
        send(conn_fd, message.c_str(), message.length(),0);
      }
      else{
        printf("couldnt' find\n");
        return NULL;
      }
    }
      else if(word == "UPDATEPOSITION"){
        char * sensor_id = (char*) malloc(1025);
        int range;
        int x_pos;
        int y_pos;
        iss >> sensor_id >> range >> x_pos >> y_pos;

        // printf("got %d %d\n", x_pos, y_pos);
        Sensor sen(sensor_id, range, x_pos, y_pos, conn_fd);

        // updating the map entry with a new Sensor
        sensors[string(sensor_id)] = sen;

        string reachable = get_reachable_message(x_pos, y_pos, range);


        send(conn_fd, reachable.c_str(), reachable.length()-1,0);
        


        // TODO
        // get list of in reach nodes
        // send it to client

      }
    if(n <= 0){
      return NULL;
    }
  }
  
}

// this waits for incoming connections
void * listen_for_new_sensors(void * arg){
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
      
      char* ID = (char*)malloc(1025 * sizeof(char));
      int range, x_pos, y_pos;
      ss >> ID; 
      ss >> range;
      ss >> x_pos;
      ss >> y_pos;


      Sensor new_sensor(ID, range, x_pos, y_pos, conn_fd);
      sensors.insert(pair<string, Sensor>(string(ID), new_sensor));
      // printf("got here\n");
      pthread_t tid;
      pthread_create(&tid, NULL, handle_single_sensor, (void*)ID);

      
      // printf("RECEIVED UPDATE POSITION\n");
    }else{

    }

    

  }
} 


void close_server(){

  map<string, Sensor>::iterator itr = sensors.begin();
  while(itr != sensors.end()){
    close(itr->second.getFD());
    itr++;
  }
  
  exit(0);
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

    // cout<<line<<endl;

    // read in contents of a line into proper variables
    string temp, base_id;
    int x_pos, y_pos, num_links;
    std::set<string> links_list;
    ss >> base_id;
    ss >> x_pos;
    ss >> y_pos;
    ss >> num_links;
    int i = 0;
    while(i < num_links ){
      ss >> temp;
      links_list.insert(temp);
      i++;
    } 

    BaseStation new_base_station(base_id, x_pos, y_pos, num_links, links_list);

    base_stations.insert(std::make_pair(base_id, new_base_station));
    // base_stations.insert(new_base_station);
    // base_station_names.insert(base_id);

    // cout << "Created new base station " + base_id << " with xpos " << x_pos << " and ypos " << y_pos<<" with " << num_links << " link(s)\n"; 
  }


  //create a server
  struct sockaddr_in address;
  int opt = 1;
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
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
  pthread_create(&tid, NULL, listen_for_new_sensors, &server_fd);


  //Wait for input from stdin
  while(true){
    getline(cin, line);
    std::istringstream iss(line);
    string word;
    iss>>word;
    if(word == "SENDDATA"){
      string origin_id;
      string dest_id;

      iss >> origin_id >> dest_id;

      string message = "THIS IS A TEST";
      int sock_fd  = sensors[origin_id].getFD();
      send(sock_fd, message.c_str(), message.length(), 0);
      // send("")

    }
    if(word == "QUIT"){
      close_server();
    }
  }
  

}
