#ifndef BSTATION_H
#define BSTATION_H

#include <string>
#include <list>

using namespace std;

class BaseStation{
  private: 
    string base_id;
    int xpos;
    int ypos;
    list<string> links_list;

  public:
    BaseStation(string base_id, int xpos, int ypos, list<string> links_list);

    const string getID() const {return base_id;}
    const int getX() const {return xpos;}
    const int getY() const {return ypos;}
    const int getNumLinks() const {return links_list.size();}
    list<string> getLinksList(){return links_list;}

    bool operator< (const BaseStation & station) const {
      if(this->getID().compare(station.getID()) < 0){
        return true;
      }
      else{
        return false;
      }
    }

};

BaseStation::BaseStation(string base_id, int xpos, int ypos, list<string> links_list){
  this->base_id = base_id;
  this->xpos = xpos;
  this->ypos = ypos;
  this->links_list = links_list;
}

class SensorBaseStation{
  private:
    string base_id;
    int xpos;
    int ypos;
    double distance_from;

  public:
    SensorBaseStation(string base_id, int xpos, int ypos);

    const string getID() const { return base_id; }
    const int getX() const { return xpos; }
    const int getY() const { return ypos; }
    const double getDist() const { return distance_from; }

    void setDist(double distance) { this->distance_from = distance;}

    bool operator< (const SensorBaseStation & station) const {
      if(this->getID().compare(station.getID()) < 0){
        return true;
      }
      else{
        return false;
      }
    }
};

SensorBaseStation::SensorBaseStation(string base_id, int xpos, int ypos){
  this->base_id = base_id;
  this->xpos = xpos;
  this->ypos = ypos;
  this->distance_from = 100;
}


#endif
