#include "unp.h"

//Structs for easier handling of each type of packet sent
typedef struct{
    uint16_t opcode;
    uint16_t block;
    char data[512];
} data_packet;

typedef struct{
    uint16_t opcode;
    uint16_t block;
} ack_packet ;

typedef struct{
    uint16_t opcode;
    uint16_t errCode;
    char errMsg[512];
} error_packet;

//global variables for tracking port numbers
int current_port;
int max_port;
int start_port;


//check available port for new connection
// if reach last port, loop around and start again from beginning
int get_new_port(){
  if(current_port < max_port){
    return current_port++;
  }else{
    current_port = start_port + 1;
    return current_port;
  }

}

// SIGCHLD
void handler(){
    pid_t pid;
    int stat;
    while((pid = waitpid(-1, &stat, WNOHANG)) > 0){
        printf("child %d terminated with status %d\n", pid, stat);
    }
}


// method to use to take in a struct of packet, and concatenate the opcode, data, 
// etc for send
void construct_message(void* packet, char* buffer, char* type){

  if(strcmp("error", type) == 0){
    printf("this is an error\n");
    error_packet * err = (error_packet*)packet;
    unsigned short * opcode_ptr = (unsigned short*) buffer;
    *opcode_ptr = err->opcode;
    *(opcode_ptr+1) = err->errCode;
    *(buffer+4) = 0;
  }

  else if(strcmp("data", type) == 0){
    printf("this is data");
  }
}

// helper function to send the error packet since it's sent so often
// called instead of creating a packet inside of the get or put request methods
void send_error(struct sockaddr_in clientaddr){

  int listenfd = -1;
  struct sockaddr_in childAddr;
  socklen_t addrlen = sizeof (struct sockaddr_in);
  error_packet err ;                 // to form the error packet
  uint16_t err_type = 1;          // stores the type of the error occured

  memset (&childAddr, 0, sizeof (struct sockaddr_in));
  memset (&err, 0, sizeof (err));


  // initialize a listenfd in here temporarily
  // TODO 
  // update functions so that they pass in listenfd instead of 
  // remaking it all the time


  // Start to send the data
  if ((listenfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0){
    perror("socket failed in sending error");
    return;
  }

  // Child server information
  childAddr.sin_family = AF_INET;
  childAddr.sin_addr.s_addr = htonl (INADDR_ANY); 
  childAddr.sin_port = htons(0);

  if (bind(listenfd, (struct sockaddr*) &childAddr, sizeof (childAddr)) < 0){
    perror("binding failed in sending error");
    return;
  }

  // error code
  err.opcode = htons(5);
  err.errCode = htons(err_type);
  char to_send[517];
  construct_message((void*)&err, to_send, "error");
  // bufferSize = sizeof (err.opcode) + sizeof (err.errCode) + strlen (err.errMsg) + 1;
  sendto (listenfd, to_send, sizeof(to_send), 0, (struct sockaddr *) &clientaddr, addrlen);
}


// function called to handle a RRQ packet
void get_request(char * fileName, struct sockaddr_in clientaddr){
  printf("Received GET request\n");
  // defining and initializing packets needed for use 
  data_packet data_pkt;
  ack_packet ack_pack;
  struct sockaddr_in child_addr;
  
  memset(&data_pkt, 0, sizeof(data_packet));
  memset(&ack_pack,0, sizeof(ack_packet));
  memset(&child_addr, 0, sizeof(struct sockaddr_in));
  
  //attempt to open file 
  FILE* fp = fopen(fileName, "r");

  //check if there was an error opening file
  if (fp == NULL){
    send_error(clientaddr);
    exit(0);
  }

  // create a new connection for the child to listen on 
  int child_fd = socket(AF_INET, SOCK_DGRAM, 0);

  if(child_fd < 0){
    perror("Socket");
    exit(0);
  }
  //initialize child sockaddr
  child_addr.sin_family = AF_INET;
  child_addr.sin_addr.s_addr = (INADDR_ANY);
  child_addr.sin_port = htons(get_new_port());

  // if the port is already taken, go to the next available one
  while(bind(child_fd, (struct sockaddr*) &child_addr, sizeof (child_addr)) < 0){
    child_addr.sin_port = htons(get_new_port());
  }

  // timeout usage, adds a timeout to the socket
  struct timeval tv;
  tv.tv_sec = 10;
  tv.tv_usec = 0;
  setsockopt(child_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  //variables used in while loop
  uint16_t block_cnt = 1;                            
  uint16_t count = 0;   
  size_t buffer_size = 0;
  uint16_t buff_len = 0;
  uint16_t retry = 0;  
  uint16_t n;
  socklen_t addrlen = sizeof (struct sockaddr_in);
  char ltr;
  while(1){
    //read in values from file and put into data packet 
    for(count = 0; count < 512; count++){
      if(fscanf(fp, "%c", &ltr) == EOF){
        break;
      }
      data_pkt.data[count] = ltr;
        
    }
    //set number for data packet and block count
    
    data_pkt.opcode = htons(3);
    data_pkt.block = htons(block_cnt % 65536);
    buffer_size = sizeof(data_pkt.opcode) + sizeof(data_pkt.block)+ (count);
    
    //communicating with client to send data
    while(1){
      //send data packet each block
      sendto(child_fd, (void*)&data_pkt, buffer_size, 0, (struct sockaddr*) &clientaddr, addrlen);
      //wait to receive and ACK packet
      n = recvfrom(child_fd, (void *)&ack_pack, sizeof (ack_pack), 0, (struct sockaddr*) &clientaddr, &addrlen);
      if(n < 0 ){
        if(errno == EINTR) continue;

        // using a counter since EWOULDBLOCK
        // blocks for 1 second, basically a psuedo timeout
        //check for RECV timeout
        if(errno == EWOULDBLOCK){
          if(++retry>=0){
            break;
          }

          // resend the packet
          sendto(child_fd, (void*)&data_pkt, buffer_size, 0, (struct sockaddr*) &clientaddr, addrlen);
        }
        perror("recvfrom");
        exit(-1);
      }
      // received normally
      else{
        break;
      }
    }

    // waited too long
    if (retry >= 10){
      printf ("ERROR: Timed out, breaking after %d tries \n", retry);
        break;
    }
    //increase block count and reset data packet for new data
    block_cnt++;
    memset(&data_pkt, 0 , sizeof( data_packet));
    retry = 0;

    //check if last packet was sent
    if ((count < 512) && (buff_len == 0)){
      break;
    }
  }
  //close FD and return
  fclose(fp);

  exit(0);

}

void put_request(char * fileName, struct sockaddr_in clientaddr){
  printf("Received PUT request\n");
  // defining and initializing packets needed for use 
  data_packet data_pkt;
  ack_packet ack_pack;
  struct sockaddr_in child_addr;

  memset(&data_pkt, 0, sizeof(data_packet));
  memset(&ack_pack,0, sizeof(ack_packet));
  memset(&child_addr, 0, sizeof(struct sockaddr_in));

  // open file for writing
  FILE* f = fopen(fileName, "w");

  if (f == NULL){
    send_error(clientaddr);
    exit(0);
  }


  // create socket connection
  int child_fd = socket(AF_INET, SOCK_DGRAM, 0);

  if(child_fd < 0){
    perror("Socket");
    exit(0);
  }

  child_addr.sin_family = AF_INET;
  child_addr.sin_addr.s_addr = (INADDR_ANY);
  child_addr.sin_port = htons(get_new_port());

  while(bind(child_fd, (struct sockaddr*) &child_addr, sizeof (child_addr)) < 0){
    child_addr.sin_port = htons(get_new_port());
  }


  // time out
  struct timeval tv;
  tv.tv_sec = 10;
  tv.tv_usec = 0;
  setsockopt(child_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  // keep track of the size of read
  uint16_t block_cnt = 0; 
  uint16_t count = 0;   
  uint16_t buff_len = 512;
  socklen_t addrlen = sizeof (struct sockaddr_in); 

  // for bytes
  int n;
  char ltr; 
  while(1){
    ack_pack.opcode = htons(4);
    ack_pack.block=block_cnt;

    // send first ack
    sendto(child_fd, (void*)&ack_pack, sizeof(ack_pack), 0, (struct sockaddr *) &clientaddr, addrlen);
    if(buff_len != 512){
      break;
    }

    // start receiving messages here
    n = recvfrom(child_fd, (void *) &data_pkt, sizeof(data_pkt), 0, (struct sockaddr *) &clientaddr, &addrlen);
    if(ntohs(data_pkt.opcode) != 3){
      continue;
    }

    //get size of buffer without opcode and block
    buff_len = n - sizeof(data_pkt.opcode) - sizeof(data_pkt.block);
    block_cnt = data_pkt.block;

    //read each character and put into file
    while (count < buff_len){
      ltr = data_pkt.data[count];
      count++;

      //check if at end of file
      if(putc(ltr, f)== EOF){
        fprintf (stderr, "Error : There was an error while writing to the file \n");
        exit(0);
      }
    }
    //reset counter and data packet for new data
    count  = 0;
    memset(&data_pkt, 0, sizeof (data_pkt));
  }

  // clean up
  fclose(f);
  exit(0);

}


int main(int argc, char ** argv){
  if(argc < 2){
        fprintf(stderr, "ERROR:Invalid number of arguments\n");
        return EXIT_FAILURE;
  }
  //read in start and end port 
  start_port = atoi(argv[1]);
  max_port = atoi(argv[2]);
  current_port = start_port;

  // interupt sigchld
  struct sigaction act;
  act.sa_handler = handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  sigaction(SIGCHLD, &act, NULL);

  //create listener socket for TFTP server
  int listenfd = socket(AF_INET, SOCK_DGRAM, 0);

  //check if socket could not be opened
  if(listenfd < 0){
    perror("Socket Error:");
    return EXIT_FAILURE;
  }
  //initialize sockaddrs
  struct sockaddr_in servaddr;
  struct sockaddr_in clientaddr;

  //set memory for sockaddr structs
  memset(&servaddr, 0, sizeof(servaddr));
  memset(&clientaddr, 0, sizeof(clientaddr));

  //Setting up server
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(start_port);

  //bind server to starting port
  if(bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr))< 0 ){
    perror("bind");
    return EXIT_FAILURE;
  }

  printf("Server up on port %d\n", ntohs(servaddr.sin_port));

  //create buffer for receiving data
  char buffer[517];
  char ip[INET_ADDRSTRLEN] = {0};
  socklen_t client_len;
  while(1){
    memset(&clientaddr, 0 , sizeof(clientaddr));
    memset(&buffer, 0, sizeof(buffer));
    client_len = sizeof(clientaddr);
    //receive a request from a client
    recvfrom(listenfd, buffer, sizeof(buffer), 0, (struct sockaddr *) & clientaddr, &client_len);
    inet_ntop( AF_INET, &(clientaddr.sin_addr), ip, INET_ADDRSTRLEN);

    
    // get the opcode and the filename to check for what request
    // and what file
    unsigned short *opcode_ptr = (unsigned short *)buffer;
    uint16_t opcode = ntohs(*opcode_ptr);

    // accept the connection
    if(opcode == 1 || opcode == 2){
      //fork to create new client and process request
      pid_t pid = fork();

      if(pid == -1){
        perror("fork");
        return EXIT_FAILURE;
      }

      if(pid == 0){
        // don't interfere with the parent
        close(listenfd);

        // get the filename
        char fileName[80];
        strcpy(fileName, buffer + 2);

        // //GET request
        if(opcode == 1){
          get_request(fileName, clientaddr);
        }
        //PUT request
        else if(opcode == 2){
          put_request(fileName, clientaddr);
        }
        //Other request, shouldn't happen
        else{
          fprintf(stderr, "ERROR: Invalid Client request\n");
        }
      }
    }
  }

  return EXIT_SUCCESS;

}
