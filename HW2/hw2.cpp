//  extern "C"{
//    #include "unp.h"
//  }
#include <iostream>
#include <algorithm>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

typedef struct {
  string username;
  int conn_fd;

} connection;

int seed;
unsigned short port;
string dic_file;
int long_word_len;
vector <connection> user_list;
int num_users = 0;

int main(int argc, char ** argv){
  if(argc < 4){
    cerr << "Error: Invalid Number of Arguments\n";
    exit(EXIT_FAILURE);
  }

  seed = atoi(argv[1]);
  port = atoi(argv[2]);
  dic_file = argv[3];
  long_word_len = atoi(argv[4]);

  //cout << "Seed is " << seed << " on port " << port << "\n";

  fd_set master_fd;
  fd_set read_fds;

  struct sockaddr_in servaddr;

  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);

  int listen_fd;

  if( (listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0 ){
    cerr << "ERROR: Socket\n";
    exit(EXIT_FAILURE);
  }

  FD_ZERO(&master_fd);
  FD_ZERO(&read_fds);

  if(bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1){
    perror("bind");
    exit(EXIT_FAILURE);
  }

  if(listen(listen_fd, 5) == -1){
     perror("listen");
     exit(EXIT_FAILURE);
  }

  FD_SET(listen_fd, &master_fd);

  cout<< "Server up on port " << ntohs(servaddr.sin_port) << "\n";

  while(1){

  }


}