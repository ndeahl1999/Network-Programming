class Sensor{


  public:

    Sensor(char* id_, int range_, int x_pos_, int y_pos_);
    ~Sensor();
    Sensor(char* id_, int range_, int x_pos_, int y_pos_, int conn_fd_);

    char* id;
    int range;
    int x_pos;
    int y_pos;
    int conn_fd;



};

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

Sensor::~Sensor(){


}
