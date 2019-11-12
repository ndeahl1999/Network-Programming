#include <string>


class Node{


  public:
    Node();
    Node(std::string _id, int x, int y, std::string type);




  private:
    std::string id;
    int xpos;
    int ypos;
    std::string type;
    SensorDetails s;
    BaseStationDetails b;



};

Node::Node(std::string _id, int x, int y, std::string type){

  if(type == "sensor"){

  }
  else if(type == "base"){

  }
}

class SensorDetails{

};

class BaseStationDetails{



};