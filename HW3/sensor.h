#include <set>
#include "base_station.h"
#include <cmath>

class Sensor{


  public:

    Sensor();
    Sensor(char* id_, int range_, int x_pos_, int y_pos_);
    ~Sensor();
    Sensor(char* id_, int range_, int x_pos_, int y_pos_, int conn_fd_);

    const char* getID() const { return id; }
    const int getRange() const { return range; }
    const int getX() const { return x_pos; }
    const int getY() const { return y_pos; }
    const int getFD() const { return conn_fd; }
    void setX(int x) { x_pos = x; }
    void setY(int y) { y_pos = y; }
    void add_in_reach(const SensorBaseStation station);
    std::set<SensorBaseStation> in_reach;

  private:
    char* id;
    int range;
    int x_pos;
    int y_pos;
    int conn_fd;

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

  // printf("coords of station %s is %d %d\n", station.getID().c_str(), station.getX(), station.getY());
  // printf("the positions in question are %f %f\n ", pow(this->x_pos-station.getX(),2), pow(this->y_pos-station.getY(),2));
  double distance = sqrt( pow( station.getX() - this->x_pos, 2) + pow(station.getY() - this->y_pos, 2));

  // printf("the distance is %f\n",distance);
  station.setDist(distance);
  in_reach.insert(station);  
  // TODO
  // maybe store this in another data structure that orders by distance so
  // future won't have to loop through entire set to find which is the closest?


}

Sensor::~Sensor(){


}
