#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include "sensor.h"




// helper function to detect if a sensor or base station is in range
bool in_range(int a_x, int a_y, int b_x, int b_y);


// 3 step process
// 1) if the intended recipient is in range, send to recipient
// 2) sort all in range sensors and base stations by distance to destination
//    by cartesian distance.
// 3) sender sends message to the base station with lowest distance to intended
//    recipient (that does not cause a cycle)
void send_message();



// can take input from std in
// move [new x pos] [new y pos]
// senddata [destination id]
// updateposition [sensorid] [sensorrange] [current x] [current y]
// quit
void handle_input();



int main(int argc, char **argv){


  char* control_address = argv[1];
  int control_port = atoi(argv[2]);
  char* sensor_id = argv[3];
  int sensor_range = atoi(argv[4]);
  int initial_x_position = atoi(argv[5]);
  int initial_y_position = atoi(argv[6]);

  Sensor s(sensor_id, sensor_range, initial_x_position, initial_y_position);


  //connect to control
  

  // send an updateposition message
  
  // start handling input
  
  handle_input();
  

  return 1;

}


void handle_input(){




}
