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


int seed;
unsigned short port;
char * dic_file;
int long_word_len;
vector <connection> user_list;
int num_users = 0;


// helper function to get amount of correct letters in any place
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
  return count;
}



// helper function to get the amount of correct letters in proper place
int correct_placement(string answer, string guess){
  if(answer.length() != guess.length()){
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
string new_guess(string username, string answer, string guess){

  int correct_chars = correct_characters(answer, guess);

  int correct_place = correct_placement(answer, guess);

  // if they guessed the wrong # of characters
  if(correct_place == -1){
    return "Invalid guess length. The secret word is " + to_string(answer.length()) + " letter(s).\n";
  } 

  return username +" guessed " + guess + ": " + to_string(correct_chars)
    +" letter(s) were correct and " + to_string(correct_place) + " letter(s)"
    + " were correctly placed.\n";
 
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


// helper function that sends a message to everyone that is connected
void send_to_all(string message){
 for(int j = 0; j< user_list.size(); j++){
    send(user_list[j].conn_fd, message.c_str(), strlen(message.c_str()), 0);
  }


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

  // main game loop
  while(1){

    // pick a new word
    string answer = words[rand() % words.size()];
    cout<<answer<<endl;

    while(1){
        printf("the length is %d\n", strlen(buffer));
      bzero(&buffer, 1025);

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

        // debug that we got a new connection
        cout << "New connection from " << inet_ntoa(client_addr.sin_addr) << " on port " << ntohs(client_addr.sin_port) << "\n"; 

        string connect_msg = "Welcome to Guess the Word, please enter your username.\n";
        send(new_sock, connect_msg.c_str(), strlen(connect_msg.c_str()), 0);

        int valid_user = 0;
        
        string username="";


        while(!valid_user){
        printf("the length is %d\n", strlen(buffer));
          bzero(&buffer, 1025);
          int n = recv(new_sock, buffer, 1025, 0);
          
          // get just the username without extra buffer
          char *temp = (char*)malloc(n * sizeof(char));
          strncpy(temp,buffer,n-1);
          string tmp_name(temp);
          valid_user = user_check(tmp_name);

          if(valid_user){

            // store a new player in the vector to prevent duplicate names
            connection new_user;
            username = tmp_name;
            new_user.username = tmp_name;
            new_user.conn_fd = new_sock;
            user_list.push_back(new_user);

          }
          // otherwise it's already been taken
          else{
            string name_error = "Username " + tmp_name + " is already taken, please enter a different username\n";
            send(new_sock, name_error.c_str(), strlen(name_error.c_str()),0);
            
          }
          
        }
        

        // send initial welcome message
        string connect_success = "Let's start playing, " + username+ "\n";
        send(new_sock, connect_success.c_str(), strlen(connect_success.c_str()), 0); 


        // send initial info message about # of players
        string players = "There are " + to_string(user_list.size())  + " player(s) playing. The secret word is " + to_string(answer.size()) + " letter(s).\n";
        send(new_sock, players.c_str(), strlen(players.c_str()), 0); 
        printf("the length is %d\n", strlen(buffer));
        bzero(&buffer, 1025);
      }

      bool correct = false;
      for(int i = 0; i < user_list.size(); i++){
        connection user = user_list[i];
      
        
        if(FD_ISSET(user.conn_fd, &read_fds)){

          // reset the buffer in case
        printf("the length is %d\n", strlen(buffer));
          bzero(&buffer, 1025);

          //recieve guess from user 
          int n = recv(user.conn_fd, buffer, 1025, 0);
          if(n >0){

            // store the guessed word, make sure not to store extra buffer
            char*guess = (char *)malloc(n* sizeof(char));
            strncpy(guess, buffer, n-1);

            string string_buffer(guess);

            string return_statement;
            printf("%s and %s\n", answer.c_str(), string_buffer.c_str());
            // if the guess is not correct
            if(answer.compare(string_buffer) != 0){

              return_statement = new_guess(user.username, answer, string_buffer);

              // if invalid size
              if(string_buffer.size() != answer.size()){
                send(user.conn_fd, return_statement.c_str(), strlen(return_statement.c_str()),0);
              }

              //if valid size but incorrect placement of chars
              else{
                send_to_all(return_statement);
              } 
             }else{//correct guess 
              return_statement = user.username +  " has correctly guessed the word " + answer + "\n";
              send_to_all(return_statement);

              // variable to know to break out of main loop
              correct=true;
              break;

            }
          
            
          }
          // if user disconnects
          else{
            close(user_list[i].conn_fd);
            user_list.erase(user_list.begin() + i);
            i--;
          }
        }
      }
      // close all connections here, and break out of the game loop
      if(correct == true){
        for(int j=user_list.size()-1;j>=0;j--){

          // erase from the end to prevent shifting while removing
          connection user = user_list[j];
          close(user.conn_fd);
          user_list.erase(user_list.begin()+j);

        }

        // pick a new word
        break;
      }


    }

  }


}
