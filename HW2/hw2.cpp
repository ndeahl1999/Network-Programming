//  extern "C"{
//    #include "unp.h"
//  }
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <set>
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

int correct_characters(const string answer, const string guess){

  map<char, int> answer_chars = map<char, int>();

  // add the counts of all characters to the map
  for(int i=0;i<answer.length();i++){
    answer_chars[answer[i]]+=1;
  }

  int count=0;

  // for every character, if it's in the map
  // increment count, and remove 1 from the map
  for(int i=0;i<guess.length();i++){
    if(answer_chars[guess[i]] != 0){
      count++;
      answer_chars[guess[i]]--;
    }
  }
  /*

  INITIAL IMPLEMENTATION, WRONGkk
  set<char> guess_chars = set<char>();

  // add the chars of guess to a set, keeping only uniques
  for(int i=0;i<guess.length();i++){
    guess_chars.insert(guess[i]);
  }

  int count =0;
  // loop through set, and if add to the count
  // the value at key c
  for(char c: guess_chars){
    count+=answer_chars[c];
    cout<<"count is "<< count <<endl;
  a

*/

  return count;
}



int correct_placement(string answer, string guess){
  if(answer.length() != guess.length()){
    cout<<"Invalid guess length. The secret word is "<<answer.length()<<" letter(s)."<<endl;
    return -1;
  }

  int count = 0;

  // count up the amount of same letters it has
  for(int i=0;i<answer.length();i++){
    if(answer[i] == guess[i]){
      count++;
    }
  }

  return count;
}

// wrapper function for the print statement
// about correct characters and placement
void new_guess(string username, string answer, string guess){

  int correct_chars = correct_characters(answer, guess);

  int correct_place = correct_placement(answer, guess);
  if(correct_place == -1){
    return;
  }

  cout<<username<<" guessed " << guess << ": "<< correct_chars
    <<" letter(s) were correct and " << correct_place<< " letter(s)"
    << " were correctly placed."<<endl;
 
}

// function repsonsible for making sure username is unique 
// returns 1 if it is
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

  ifstream infile(dic_file);
  string line;
  vector<string> words;

  // read in all the words
  while(infile >> line){
    words.push_back(line);
  }

  srand(seed);



  string answer = words[rand() % words.size()];

  cout<< answer<<endl;


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

      
      string username="";
      while(!valid_user){
        int n = recv(new_sock, buffer, sizeof(buffer), 0);
        char *temp = (char*)malloc(n * sizeof(char));
        for (int i=0; i<512; i++) {
          printf("%02x ", buffer[i]);
          if ((i+1)%16 == 0) printf("\n");
        }
        strncpy(temp,buffer,n);
        string tmp_name = string(temp);
        valid_user = user_check(tmp_name);

        if(valid_user){
          connection new_user;
          //tmp_name = tmp_name.substr(0, tmp_name.size()-1);
          username = tmp_name;
          new_user.username = tmp_name;
          new_user.conn_fd = new_sock;
          user_list.push_back(new_user);

        }else{
          
          string name_error = "Username " + tmp_name + " is already taken, please enter a different username\n";
          send(new_sock, name_error.c_str(), strlen(name_error.c_str()),0);
          
        }
        
      }
      
      cout<<"secret is "<<answer<<endl;
      cout<<"sending first"<<endl;
      string connect_success = "Let's start playing, " + username+ "\n";
      send(new_sock, connect_success.c_str(), strlen(connect_success.c_str()), 0); 

      cout<<"sending second"<<endl;

      string players = "There are " + to_string(user_list.size())  + " player(s) playing. The secret word is " + to_string(answer.size()) + " letter(s).\n";
      printf("%s\n", players.c_str());
      // cout<<players<<endl;
      send(new_sock, players.c_str(), strlen(players.c_str()), 0); 
      bzero(&buffer, sizeof(buffer));
    }

    for(int i = 0; i < user_list.size(); i++){
      connection user = user_list[i];
    

      if(FD_ISSET(user.conn_fd, &read_fds)){
        //recieve guess from user 
        if(recv(user.conn_fd, &buffer, sizeof(buffer), 0) >0){
          // cout<< "Received guess " << buffer << " from " << user.username << "\n";

          string string_buffer = string(buffer);
          //string_buffer = string_buffer.substr(0, string_buffer.size()-1);
          new_guess(user.username, answer, string_buffer);

          string return_guess = "You guessed " + string_buffer + "\n";



          send(user.conn_fd, return_guess.c_str(),strlen(return_guess.c_str()), 0);
        }



      }

    }

  }


}
