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

typedef struct {
  int x;
  int y;
} Pair;


bool in_range(int station_x, int station_y, int sensor_x, int sensor_y, int max_dist){

  double distance = sqrt( pow(station_x - sensor_x, 2) + pow(station_y - sensor_y, 2));
  // printf("the distance is %f and max is %d\n", distance, max_dist);
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


string get_coordinates(string target){

  string message = "THERE ";
  if(base_stations.find(target) != base_stations.end()){
    // printf("got it from base statopms\n");
    for(std::map<string, BaseStation>::iterator it = base_stations.begin();it != base_stations.end(); it++ ){
      if(it->second.getID() == target){
        // string message = "THERE ";
        message += to_string(it->second.getX());
        message += " ";
        message += to_string(it->second.getY());
        // send(conn_fd, message.c_str(), message.length(), 0);

      }
    }
  }
  else if(sensors.find(target) != sensors.end()){
    // printf("got it from sensors\n");
    // string message = "THERE ";
    message += to_string(sensors[target].getX());
    message += " ";
    message += to_string(sensors[target].getY());
    // send(conn_fd, message.c_str(), message.length(),0);
  }
  else{
    printf("couldnt' find\n");
    return NULL;
  }
  return message;
}

Pair coordinates_of(string target){

  map<string, BaseStation>::iterator it = base_stations.find(target);
  map<string, Sensor>::iterator itr = sensors.find(target);
  if(it != base_stations.end()){
    BaseStation* current = &(it->second);
    Pair p;
    p.x = current->getX();
    p.y = current->getY();

    return p;
  }
  else if(itr != sensors.end()){
    Sensor* current = &(itr->second);
    Pair p;
    p.x = current->getX();
    p.y = current->getY();

    return p;
  }

  Pair p;
  p.x = -1;
  p.y = -1;
  return p;
  // Pair p;
  
}
// string find_next_hop(string base_id, string dest_id, std::list<string>hop_list){
//   BaseStation *curr_b = &(base_stations[base_id]);
//   BaseStation *dest_b = &(base_stations[dest_id]);

//   set<string> dest_list  = dest_b->getLinksList();
//   if(dest_list.find(base_id) != dest_list.end()){
//     return dest_id;
//   }


//   return "Nothing";
// }

string get_reachable_message(string id, int a_x, int a_y, int range){
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
  for(std::pair<string, Sensor> entry: sensors){
    Sensor *s = &(entry.second);

    string temp = s->getID();
    // printf("%s, %d, %d, %d, %d\n", temp.c_str(), s->getX(), s->getY(), a_x, a_y);

    if(s->getID() == id){
      continue;
    }
    if(in_range(s->getX(), s->getY(), a_x, a_y, range)){

      
      reachable_list+=s->getID();
      reachable_list+= " ";
      reachable_list+=to_string(s->getX());
      reachable_list+= " ";
      reachable_list+=to_string(s->getY());
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
  string reachable = get_reachable_message(s.getID(), s.getX(), s.getY(), s.getRange());


  send(conn_fd, reachable.c_str(), reachable.length(),0);

  int n;
  char buffer[1025];
  

  // main loop waiting for messages
  while(true){
    bzero(&buffer, 1025);

    // wait for a recv from self
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
      std::vector<string> hop_list;

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


        // check here if it's a sensor, if it is, no need to go do below logic    

              // this means the next one to handle the message is base station
        std::map<string, BaseStation>::iterator it = base_stations.find(next_id);
        // // while((it = base_stations.find(next_id)) != base_stations.end()){


        //   // if its a base station to handle
          BaseStation *next = &(it->second);


          Pair p = coordinates_of(dest_id);
          // printf("got p %d %d\n", p.x, p.y);
        


          // check if it has the target in reach

          // check the neighbors and traverse the list
          // else{

            // continue traversing base stations until end
            while(true){
              

              // std::map<string, BaseStation>::iterator itr = base_stations.find(current->getID());
              //if a base station is next
              // if(current->getID() == dest_id){

                // BaseStation* current = &((base_stations.find(current->getID())->second));

              //add string for hop to current base station 
              printf("%s: Message from %s to %s being forwarded through %s\n", next_id.c_str(), origin_id.c_str(), dest_id.c_str(), next_id.c_str());

                if(next->canConnect(dest_id)){
                  printf("%s: Message from %s to %s successfully received.\n",dest_id.c_str(), origin_id.c_str(), dest_id.c_str());
                  break;
                }
              // }

              double min_dist = 10000;
              string min_hop = "";
              set<string> neighbors = next->getLinksList();

              // for all possible neighbors
              for(set<string>::iterator it = neighbors.begin(); it != neighbors.end(); it++){

                // if the hop list doesn't contain THIS neighbor, get if its the closest
                std::vector<string>::iterator to_jump_iterator = std::find(hop_list.begin(), hop_list.end(), *it);

                // if that neighbor is not already in the hoplist
                // reached end of hop list and couldn't find it
                if(to_jump_iterator == hop_list.end()){

                  // get dist
                  BaseStation* temp = &(base_stations[*(it)]);
                  // printf("now x:%d y:%d compared to x:%d y:%d\n", p.x, p.y, temp->getX(), temp->getY());
                  double distance = sqrt( pow( temp->getX() - p.x, 2) + pow(temp->getY() - p.y, 2));
                  // printf("%s: got %f when comparing with %s\n", current->getID().c_str(), distance, it->c_str());

                  // set dist
                  if(distance < min_dist){
                    min_dist = distance;
                    min_hop = *(it);
                  }

                }
                
              }

              // for all sensors
              for(map<string, Sensor>::iterator it = sensors.begin(); it!= sensors.end(); it++){
                Sensor* temp = &(it->second);

                // make sure the sensor is reachable from base station to start
                double distance_to_sensor_from_here = sqrt( pow( next->getX() - temp->getX(), 2) + pow( next->getY() -temp->getY(), 2));
                if(distance_to_sensor_from_here > temp->getRange()){
                  // printf("this goes off now for %s  to %s because %f\n", temp->getID(), next_id.c_str(), distance_to_sensor_from_here);
                  continue;
                }
                  // printf("now x:%d y:%d compared to x:%d y:%d\n", p.x, p.y, temp->getX(), temp->getY());
                double distance = sqrt( pow( temp->getX() - p.x, 2) + pow(temp->getY() - p.y, 2));
                  // printf("%s: got %f when comparing with %s\n", current->getID().c_str(), distance, it->second.getID());

                if(distance < min_dist){
                    min_dist = distance;
                    min_hop = it->first;
                  }
                
              }

              // now the closest hop is set
              if(min_hop == ""){
                printf("handle no other base stations here\n");
                break;
              }

              // printf("min_hop is %s\n", min_hop.c_str());

              std::map<string, BaseStation>::iterator it = base_stations.find(min_hop);
              //if a base station is next
              if(it != base_stations.end()){

                BaseStation* current = &((base_stations.find(min_hop)->second));

                if(current->canConnect(dest_id)){
                  printf("%s: Message from %s to %s successfully received.\n",dest_id.c_str(), origin_id.c_str(), dest_id.c_str());
                  break;
                }

                // add to hop list
                next_id = current->getID();

                std::map<string, BaseStation>::iterator it = base_stations.find(next_id);
                // printf(" --- %s\n", next_id.c_str());
                next = &(it->second);
                hop_list.push_back(next_id);
              }
              // otherwise a sensor is next
              else{
                Sensor* current = &((sensors.find(min_hop)->second));

                next_id = current->getID();
                // hop_list.push_back(next_id);

                string message  = "DATAMESSAGE ";
                message+=origin_id;
                message+=" ";
                message+=next_id;
                message+=" ";
                message+=dest_id;
                message+=" ";
                message+=to_string(hop_list.size());
                message+=" ";
                for(int i=0;i<hop_list.size();i++){
                  message+=hop_list[i];
                  message+=" ";
                }




                send(current->getFD(), message.c_str(), message.length(), 0);

                break;


              }
              

            // if hop list contains all possible neighbors

            // otherwise, not at end

            }

            // Pair p = coordinates_of(dest_id);

            // // bool is_base_station = false;
            // BaseStation* closest_base_station;
            // // Sensor* closest_sensor;

            // double lowest_dist = 1000000;
            // for(map<string, BaseStation>::iterator it = base_stations.begin(); it != base_stations.end(); it++){

            //   double distance = sqrt( pow( it->second.getX() - p.x, 2) + pow(it->second.getY() - p.y, 2));
            //   if(distance < lowest_dist){
            //     closest_base_station = &(it->second);
            //     lowest_dist = distance; 
            //     // is_base_station = true;
            //   }
            // }

            // for(map<string, Sensor>::iterator it = sensors.begin(); it != sensors.end(); it++){

            //   double distance = sqrt( pow( it->second.getX() - p.x, 2) + pow(it->second.getY() - p.y, 2));
            //   if(distance < lowest_dist){
            //     closest_sensor = &(it->second);
            //     lowest_dist = distance; 
            //     is_base_station = false;
            //   }
            // }



          // }

        //   // else find neighbor that is closest to that target
        //   else{

        //     // 
        //     if(base_stations.find(dest_id) != base_stations.end()){

        //     }
        //     else if(sensors.find(dest_id) != sensors.end()){

        //     }

        //   }
        }
        

        // string next_hop = find_next_hop(next_id, dest_id, hop_list);
        // string new_dm = "DATAMESSAGE " + origin_id + " " + next_hop + " " + to_string(hop_list.size()); 
        // for(string base_name : hop_list){
        //   new_dm += " " + base_name;
        // }
        // new_dm += " 0";
        // printf("--%s\n", new_dm.c_str());

        // new_dm += "";
        // send(server_fd, new_dm.c_str(), new_dm.length(),0);

    }
    else if(word == "WHERE"){
      string target;
      iss >> target;
      string message = get_coordinates(target);
      send(conn_fd, message.c_str(), message.length(),0 );
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

        string reachable = get_reachable_message("", x_pos, y_pos, range);


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
    // printf("Base station %s with x:%d y%d\n", base_id.c_str(), x_pos, y_pos);

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

      // origin, next, destination, hop length, [hop list]
      string message = "DATAMESSAGE ";
      int sock_fd  = sensors[origin_id].getFD();
      send(sock_fd, message.c_str(), message.length(), 0);
      // send("")

    }
    if(word == "QUIT"){
      close_server();
    }
  }
  

}
