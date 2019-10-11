//  extern "C"{
//    #include "unp.h"
//  }
#include <iostream>
#include <algorithm>
#include <vector>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

typedef struct {
  string username;
  int conn_fd;

} connection;

string connect_msg = "Welcome to Guess the Word, please enter your username.\n";

int seed;
unsigned short port;
char * dic_file;
int long_word_len;
vector <connection> user_list;
int num_users = 0;

int user_check(string username){
  transform(username.begin(), username.end(), username.begin(), ::tolower);
  for(int i = 0; i < user_list.size(); i++){
    string c_username = user_list[i].username;
    transform(c_username.begin(), c_username.end(), c_username.begin(), ::tolower);
    if(c_username.compare(username) == 0){
      return 0;
    }
  }
  
  return 1;
}

int main(int argc, char ** argv){
  //check for correct number fo arguments
  if(argc < 4){
    cerr << "Error: Invalid Number of Arguments\n";
    exit(EXIT_FAILURE);
  }

  //parse command-line arguments
  seed = atoi(argv[1]);
  port = atoi(argv[2]);
  dic_file = argv[3];
  long_word_len = atoi(argv[4]);

  //cout << "Seed is " << seed << " on port " << port << "\n";

  //initialize server
  struct sockaddr_in servaddr;

  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);

  int listen_fd;
  //create listener socket
  if( (listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0 ){
    cerr << "ERROR: Socket\n";
    exit(EXIT_FAILURE);
  }

  //make sure socket is set to allow multiple connections
  int opt = 1;
  if( setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char *)& opt, sizeof(opt))<0){
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }
  
  //bind listener socket to port
  if(bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
    perror("bind");
    exit(EXIT_FAILURE);
  }

  //list for up to 5 incoming connections
  if(listen(listen_fd, 5) == -1){
     perror("listen");
     exit(EXIT_FAILURE);
  }

  fd_set read_fds;
  

  cout<< "Server up on port " << ntohs(servaddr.sin_port) << "\n";

  char buffer[1025];
  int max_sd;


  while(1){
    //clear the socket set
    FD_ZERO(&read_fds);
    //add master socket to set
    FD_SET(listen_fd, &read_fds);
    max_sd = listen_fd;

    //check and set all listener ports
    // including the user connections
    for(int i = 0; i < user_list.size(); i++){
      int sd = user_list[i].conn_fd;
      //check if the file descriptor is set and add to socket set
      if(sd > 0)
        FD_SET(sd, &read_fds);
      
      //necessary for select
      if(sd > max_sd )
        max_sd = sd;
    }

    //select call for incoming connections. 
    int retval = select(max_sd + 1, &read_fds, NULL, NULL, NULL);

    if ((retval < 0) && (errno!=EINTR)){   
      cerr << "select error";   
    } 

    //if the listener socket is set, then there
    // is a new connection coming in. 
    if(FD_ISSET(listen_fd, &read_fds)){
      struct sockaddr_in client_addr;
      int new_sock; 
      int addrlen = sizeof(client_addr);

      if( (new_sock = accept(listen_fd, (struct sockaddr *) &client_addr, (socklen_t*) &addrlen))<0){
        perror("accept");
        exit(EXIT_FAILURE);
      }

      cout << "New connection from " << inet_ntoa(client_addr.sin_addr) << " on port " << ntohs(client_addr.sin_port) << "\n"; 

      send(new_sock, connect_msg.c_str(), strlen(connect_msg.c_str()), 0);

      int valid_user = 0;

      while(!valid_user){
        int n = recv(new_sock, buffer, sizeof(buffer), 0);
        string tmp_name = string(buffer);

        valid_user = user_check(tmp_name);

        if(valid_user){
          connection new_user;
          new_user.username = tmp_name;
          new_user.conn_fd = new_sock;
          user_list.push_back(new_user);

        }else{
          
          string name_error = "Username " + tmp_name + " is already taken, please enter a different username\n";
          send(new_sock, name_error.c_str(), strlen(name_error.c_str()),0);
          
        }
      }

      string connect_success = "Let's start playing, " + string(buffer)+ "\n";
      send(new_sock, connect_success.c_str(), strlen(connect_success.c_str()), 0); 
    }

    for(int i = 0; i < user_list.size(); i++){
      connection user = user_list[i];

      if(FD_ISSET(user.conn_fd, &read_fds)){
        //recieve guess from user 


      }

    }

  }


}
