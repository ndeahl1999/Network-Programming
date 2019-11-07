#include <set>
#include "base_station.h"
#include <cmath>

class Sensor{


  public:

    Sensor();
    Sensor(char* id_, int range_, int x_pos_, int y_pos_);
    ~Sensor();
    Sensor(char* id_, int range_, int x_pos_, int y_pos_, int conn_fd_);

    void add_in_reach(const SensorBaseStation station);

    char* id;
    int range;
    int x_pos;
    int y_pos;
    int conn_fd;
    std::set<SensorBaseStation> in_reach;

};

// this should only be used at the beginnig but be reassinged 
Sensor::Sensor(){

}
Sensor::Sensor(char* id_, int range_, int x_pos_, int y_pos_){
  id = id_;
  range = range_;
  x_pos = x_pos_;
  y_pos = y_pos_;
}


Sensor::Sensor(char* id_, int range_, int x_pos_, int y_pos_, int conn_fd_){
  id = id_;
  range = range_;
  x_pos = x_pos_;
  y_pos = y_pos_;
  conn_fd = conn_fd_;
}

void Sensor::add_in_reach(SensorBaseStation station){

  double distance = sqrt( pow(this->x_pos - station.getX(), 2) + pow(this->y_pos - station.getY(), 2));

  station.setDist(distance);
  in_reach.insert(station);  
  // TODO
  // maybe store this in another data structure that orders by distance so
  // future won't have to loop through entire set to find which is the closest?


}

Sensor::~Sensor(){


}
