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

    string getID(){return base_id;}
    int getX(){return xpos;}
    int getY(){return ypos;}
    int getNumLinks(){return links_list.size();}
    list<string> getLinksList(){return links_list;}

};

  BaseStation::BaseStation(string base_id, int xpos, int ypos, list<string> links_list){
    this->base_id = base_id;
    this->xpos = xpos;
    this->ypos = ypos;
    this->links_list = links_list;
  }

#endif
