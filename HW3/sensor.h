class Sensor{


  public:

    Sensor(char* id_, int range_, int x_pos_, int y_pos_);
    ~Sensor();


    char* id;
    int range;
    int x_pos;
    int y_pos;









};

Sensor::Sensor(char* id_, int range_, int x_pos_, int y_pos_){
  id = id_;
  range = range_;
  x_pos = x_pos_;
  y_pos = y_pos_;
}

Sensor::~Sensor(){


}
