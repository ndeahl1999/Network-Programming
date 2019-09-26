#include "unp.h"
#include <stdbool.h>

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

// helper function to send the error packet since it's sent so often
// called instead of creating a packet inside of the get or put request methods
void send_error(struct sockaddr_in clientaddr){

  int child_fd = -1;
  struct sockaddr_in childAddr;
  socklen_t addrlen = sizeof (struct sockaddr_in);
  error_packet err ;                 // to form the error packet
  int errType = 1;          // stores the type of the error occured
  int bufferSize = 0;

  memset (&childAddr, 0, sizeof (struct sockaddr_in));
  memset (&err, 0, sizeof (err));

  // Start to send the data
  if ((child_fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0){
    perror("socket failed in sending error");
    return;
  }

  // Child server information
  childAddr.sin_family = AF_INET;
  childAddr.sin_addr.s_addr = htonl (INADDR_ANY); 
  childAddr.sin_port = htons(0);

  if (bind(child_fd, (struct sockaddr*) &childAddr, sizeof (childAddr)) < 0){
    perror("binding failed in sending error");
    return;
  }

  err.opcode = htons(5);
  err.errCode = htons(errType);
  bufferSize = sizeof (err.opcode) + sizeof (err.errCode) + strlen (err.errMsg) + 1;
  sendto (child_fd, err.errMsg, bufferSize, 0, (struct sockaddr *) &clientaddr, addrlen);
}


// function called to handle a RRQ packet
int get_request(char * fileName, struct sockaddr_in clientaddr){
  printf("Received GET request\n");
  data_packet data_pkt;
  ack_packet ack_pkt;
  struct sockaddr_in child_addr;
  memset(&data_pkt, 0, sizeof(data_packet));
  memset(&ack_pkt,0, sizeof(ack_packet));
  memset(&child_addr, 0, sizeof(struct sockaddr_in));
  
  FILE* f = fopen(fileName, "r");

  if (f == NULL){
    send_error(clientaddr);
    exit(0);
  }

  int child_fd = socket(AF_INET, SOCK_DGRAM, 0);

  if(child_fd < 0){
    perror("Socket");
    exit(0);
  }

  child_addr.sin_family = AF_INET;
  child_addr.sin_addr.s_addr = (INADDR_ANY);
  child_addr.sin_port = htons(get_new_port());

  // port unavailable, try another 1...
  while(bind(child_fd, (struct sockaddr*) &child_addr, sizeof (child_addr)) < 0){
    child_addr.sin_port = htons(get_new_port());
  }
  
  fd_set read_fds;
  FD_ZERO (&read_fds);
  FD_SET (child_fd, &read_fds);

  struct timeval tv;
  tv.tv_sec = 10;
  tv.tv_usec = 0;


  uint16_t block_cnt = 1;
  uint16_t count = 0;   
  size_t buffer_size = 0;
  bool isEOF = false; 
  uint16_t buff_len = 0;
  char buffer [512] = {0};
  uint16_t buff_index = 0;
  int total_data = 0;
  char next_char = -1;    
  char character = '\0'; 
  int retry_count = 0;  
  int status = 0;
  socklen_t addrlen = sizeof (struct sockaddr_in);
  while(1){
    for(count = 0; count < 512; count++){
        if((buff_index == buff_len) && (isEOF == false)){
          memset(buffer, 0, sizeof(buffer));
          buff_index = 0;
          buff_len= fread(buffer, 1, sizeof(buffer), f );
          total_data += buff_len;
          
          if (buff_len < 512) {
            isEOF = true;
          }

          if (ferror (f))
          {
            fprintf(stderr, "ERROR: Read error from getc on local machine\n");
          } 
        }
        if(next_char >= 0){
          data_pkt.data [count]= next_char;
          next_char = -1;
          continue;
        }
        if (buff_index >= buff_len)
        {
          break;
        }
        character = buffer[buff_index];
        buff_index++;
        data_pkt.data[count] = character;
    }
    data_pkt.opcode = htons(3);
    data_pkt.block = htons(block_cnt % 65536);
    buffer_size = sizeof(data_pkt.opcode) + sizeof(data_pkt.block)+ (count);
    do{
      sendto(child_fd, (void*)&data_pkt, buffer_size, 0, (struct sockaddr*) &clientaddr, addrlen);
      if((status= select (child_fd + 1, &read_fds, NULL, NULL, &tv) <= 0)){
        retry_count++;
        continue;
      }else{
        if((status = recvfrom (child_fd, (void *)&ack_pkt, sizeof (ack_pkt), 0, (struct sockaddr*) &clientaddr, &addrlen)) >= 0){
          if((ntohs(ack_pkt.opcode )== 4) && (ntohs(ack_pkt.block)== block_cnt)){
            break;
          }
        }
      }
      memset (&ack_pkt, 0, sizeof (ack_packet));
    }while(retry_count < 10);
    if (retry_count >= 10)
    {
      printf ("ERROR: Timed out, breaking after %d tries \n", retry_count);
        break;
    }
    block_cnt++;
    memset(&data_pkt, 0 , sizeof( data_packet));
    retry_count = 0;
    if ((count < 512) && (buff_index == buff_len)){
      break;
    }
  }

  fclose(f);

  return 1;

}

int put_request(char * fileName, struct sockaddr_in clientaddr){
  printf("Received PUT request\n");
  data_packet data_pkt;
  ack_packet ack_pkt;
  struct sockaddr_in child_addr;
  memset(&data_pkt, 0, sizeof(data_packet));
  memset(&ack_pkt,0, sizeof(ack_packet));
  memset(&child_addr, 0, sizeof(struct sockaddr_in));

  FILE* f = fopen(fileName, "w");

  if (f == NULL)
  {
    send_error(clientaddr);
    return 0;
  }


  int child_fd = socket(AF_INET, SOCK_DGRAM, 0);

  if(child_fd < 0){
    perror("Socket");
    return 0;
  }

  child_addr.sin_family = AF_INET;

  child_addr.sin_addr.s_addr = (INADDR_ANY);

  child_addr.sin_port = htons(get_new_port());

  while(bind(child_fd, (struct sockaddr*) &child_addr, sizeof (child_addr)) < 0)
  {
    child_addr.sin_port = htons(get_new_port());
  }
  
  fd_set read_fds;
  FD_ZERO (&read_fds);
  FD_SET (child_fd, &read_fds);

  struct timeval tv;
  tv.tv_sec = 10;
  tv.tv_usec = 0;

  uint16_t block_cnt = 0; 
  uint16_t count = 0;   
  size_t buffer_size = 0;
  bool error_ocr = false; 
  uint16_t buff_len = 512;
  uint16_t byte_len = 0;  
  char character = '\0'; 
  int retry_count = 0;  
  int status = 0;
  bool lastCharWasCR = false;
  socklen_t addrlen = sizeof (struct sockaddr_in); 
  
  
  while(1){
    ack_pkt.opcode = htons(4);
    ack_pkt.block=block_cnt;
    sendto(child_fd, (void*)&ack_pkt, sizeof(ack_pkt), 0, (struct sockaddr *) &clientaddr, addrlen);
    if((buff_len != 512) || (error_ocr == true)){
      break;
    }
    if((status = select (child_fd + 1, &read_fds, NULL, NULL, &tv)) <= 0){
      if(status == 0){
        fprintf(stderr, "Timedout while waiting for packet from the host \n");
        break;
      }else if (status < 0){
        perror("select");
        break;
      }
    }else{
      if((byte_len = recvfrom(child_fd, (void *) &data_pkt, sizeof(data_pkt), 0, (struct sockaddr *) &clientaddr, &addrlen)) >= 0){
        if(ntohs(data_pkt.opcode) != 3){
          continue;
        }
        buff_len = byte_len - sizeof(data_pkt.opcode) - sizeof(data_pkt.block);
        block_cnt = data_pkt.block;
        while (count < buff_len){
          character = data_pkt.data[count];
          count++;


          if(putc(character, f)== EOF){
            fprintf (stderr, "Error : There was an error while writing to the file \n");
            error_ocr = true;
            break;
          }
        }
        count  = 0;
        memset(&data_pkt, 0, sizeof (data_pkt));
      }
    }

  }

  fclose(f);
  return 1;

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

  if(listenfd < 0){
    perror("Socket Error:");
    return EXIT_FAILURE;
  }
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
  int bindcheck= bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

  if(bindcheck < 0 ){
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
    int opcode = ntohs(*opcode_ptr);

    // accept the connection
    if(opcode == 1 || opcode == 2){
      //fork to create new client and process request
      pid_t pid = fork();

      if(pid == -1){
        perror("fork");
        return EXIT_FAILURE;
      }

      if(pid == 0){
        char fileName[80];

        // don't interfere with the parent
        close(listenfd);

        printf("%s\n", buffer + 2);
        // break;

        strcpy(fileName, buffer + 2);

        // //GET request
        if(opcode == 1){
          get_request(fileName, clientaddr);
        }
        //PUT request
        else if(opcode == 2){
          put_request(fileName, clientaddr);
        }
        //Other request
        else{
          fprintf(stderr, "ERROR: Invalid Client request\n");
        }
      }
    }
  }

  return EXIT_SUCCESS;

}
