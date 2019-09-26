#include "unp.h"
#include <stdbool.h>

//Structs for easier handling of each type of packet sent

struct REQPKT
{
    uint16_t opcode;
    char fileName[1024];
};

struct DATAPKT
{
    uint16_t opcode;
    uint16_t blkNumber;
    char data[512];
};

struct ACKPKT
{
    uint16_t opcode;
    uint16_t blkNumber;
};

struct ERRPKT
{
    uint16_t opcode;
    uint16_t errCode;
    char errMsg[512];
};

//global variables for tracking port numbers;
int current_port;
int max_port;
int start_port;


//check available port for new connection;
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
        printf("child %d terminatied\n", pid);

        if(WIFEXITED(stat)){
          printf("Child exited with code: %d\n", WEXITSTATUS(stat));
        }
    }
}

void send_error(int errNumber, struct sockaddr_in clientaddr){

    int childFd = 0;
    struct sockaddr_in childAddr;
    socklen_t addrlen = sizeof (struct sockaddr_in);
    struct ERRPKT errPkt ;                 // to form the error packet
    int errType = 1;          // stores the type of the error occured
    int bufferSize = 0;

    memset (&childAddr, 0, sizeof (struct sockaddr_in));
    memset (&errPkt, 0, sizeof (struct ERRPKT));

    // Start to send the data
    if ((childFd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        // printErrMsg ("socket");
        return;
    }

    // Child server information
    childAddr.sin_family = AF_INET;
    childAddr.sin_addr.s_addr = htonl (INADDR_ANY); // Taking local IP
    childAddr.sin_port = htons(0);                  // Bind will assign a ephemeral port

    if (bind(childFd, (struct sockaddr*) &childAddr, sizeof (childAddr)) < 0)
    {
        // printErrMsg ("bind");

        return;
    }

    errPkt.opcode = htons(5);
    if (errNumber == EACCES)
    {
        // errType = ACCESS_VIOLATION;
    }
    errPkt.errCode = htons(errType);
    snprintf (errPkt.errMsg, sizeof (errPkt.errMsg), "%c", errPkt.errMsg[errType]);
    bufferSize = sizeof (errPkt.opcode) + sizeof (errPkt.errCode) + strlen (errPkt.errMsg) + 1;
    sendto (childFd, (void *)(&errPkt), bufferSize, 0, (struct sockaddr *) &clientaddr, addrlen);
}

int get_request(char * fileName, struct sockaddr_in clientaddr){
  struct DATAPKT data_pkt;
  struct ACKPKT ack_pkt;
  struct sockaddr_in child_addr;
  memset(&data_pkt, 0, sizeof(struct DATAPKT));
  memset(&ack_pkt,0, sizeof(struct ACKPKT));
  memset(&child_addr, 0, sizeof(struct sockaddr_in));
  
  FILE* fp = fopen(fileName, "r");

  if (fp == NULL)
  {
    send_error(errno, clientaddr);
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


  uint16_t block_cnt = 1;                            // Block count will have the block number that needs to be sent next in the pkt
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
          buff_len= fread(buffer, 1, sizeof(buffer), fp );
          total_data += buff_len;
          
          if (buff_len < 512) {
            isEOF = true;
          }

          if (ferror (fp))
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
    data_pkt.blkNumber = htons(block_cnt % 65536);
    buffer_size = sizeof(data_pkt.opcode) + sizeof(data_pkt.blkNumber)+ (count);
    do{
      sendto(child_fd, (void*)&data_pkt, buffer_size, 0, (struct sockaddr*) &clientaddr, addrlen);
      if((status= select (child_fd + 1, &read_fds, NULL, NULL, &tv) <= 0)){
        retry_count++;
        continue;
      }else{
        if((status = recvfrom (child_fd, (void *)&ack_pkt, sizeof (ack_pkt), 0, (struct sockaddr*) &clientaddr, &addrlen)) >= 0){
          if((ntohs(ack_pkt.opcode )== 4) && (ntohs(ack_pkt.blkNumber)== block_cnt)){
            break;
          }
        }
      }
      memset (&ack_pkt, 0, sizeof (struct ACKPKT));
    }while(retry_count < 10);
    if (retry_count >= 10)
    {
      printf ("ERROR: Timed out, breaking after %d tries \n", retry_count);
        break;
    }
    block_cnt++;
    memset(&data_pkt, 0 , sizeof( struct DATAPKT));
    retry_count = 0;
    if ((count < 512) && (buff_index == buff_len)){
      break;
    }
  }

  fclose(fp);

  return 1;

}

int put_request(char * fileName, struct sockaddr_in clientaddr){
  struct DATAPKT data_pkt;
  struct ACKPKT ack_pkt;
  struct sockaddr_in child_addr;
  memset(&data_pkt, 0, sizeof(struct DATAPKT));
  memset(&ack_pkt,0, sizeof(struct ACKPKT));
  memset(&child_addr, 0, sizeof(struct sockaddr_in));

  FILE* fp = fopen(fileName, "r");

  if (fp == NULL)
  {
    send_error(errno, clientaddr);
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

  uint16_t block_cnt = 1; 
  uint16_t count = 0;   
  size_t buffer_size = 0;
  bool error_ocr = false; 
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
    ack_pkt.opcode = htons(4);
    ack_pkt.blkNumber=block_cnt;
    sendto(child_fd, (void*)&ack_pkt, sizeof(ack_pkt), 0, (struct sockaddr *) &clientaddr, addrlen);
    if((buff_len != 512) || (error_ocr == true)){
      break;
    }
    if((status = select (child_fd + 1, &read_fds, NULL, NULL, &tv)) <= 0){
      if(status == 0){
        fprintf(stderr, "Timedout while waiting for packet from the host \n");
        break;
      }else if ( status>0){
        perror("select");
        break;
      }
    }

  }



}

void process_request(void* buffer, struct sockaddr_in clientaddr){
  int req_type = 0;
  struct REQPKT * req_pkt = buffer;

  req_pkt->opcode = ntohs(*((uint16_t*)buffer));
  //GET request
  if(req_pkt->opcode == 1){
    get_request(req_pkt->fileName, clientaddr);
  //PUT request
  }else if(req_pkt->opcode == 2){
    put_request(req_pkt->fileName, clientaddr);
  //Other request
  }else{
    fprintf(stderr, "ERROR: Invalid Client request\n");
  }

}


int main(int argc, char ** argv){
  if(argc < 2){
        fprintf(stderr, "ERROR:Invalid number of arguments\n");
        return EXIT_FAILURE;
  }

  start_port = atoi(argv[1]);
  max_port = atoi(argv[2]);
  current_port = start_port;

  // interupt sigchld
  struct sigaction act;
  act.sa_handler = handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  sigaction(SIGCHLD, &act, NULL);


  int listenfd = socket(AF_INET, SOCK_DGRAM, 0);

  if(listenfd < 0){
    perror("Socket Error:");
    return EXIT_FAILURE;
  }
  struct sockaddr_in servaddr;
  struct sockaddr_in clientaddr;

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&clientaddr, 0, sizeof(clientaddr));

  //Setting up server
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(start_port);

  int bindcheck= bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

  if(bindcheck < 0 ){
    perror("bind");
    return EXIT_FAILURE;
  }

   printf("Server up on port %d\n", ntohs(servaddr.sin_port));

  char buffer[517];
  char ip[INET_ADDRSTRLEN] = {0};
  socklen_t client_len;
  while(1){
    memset(&clientaddr, 0 , sizeof(clientaddr));
    memset(&buffer, 0, sizeof(buffer));
    client_len = sizeof(clientaddr);
    recvfrom(listenfd, (void *) buffer, sizeof(buffer), 0, (struct sockaddr *) & clientaddr, &client_len);
    inet_ntop( AF_INET, &(clientaddr.sin_addr), ip, INET_ADDRSTRLEN);
    pid_t pid = fork();

    if(pid == -1){
      perror("fork");
      return EXIT_FAILURE;
    }

    if(pid == 0){
      process_request((void *) buffer, clientaddr);
    }


  }

  return EXIT_SUCCESS;

}
