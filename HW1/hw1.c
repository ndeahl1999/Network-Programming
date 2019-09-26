#include "unp.h"

//Structs for easier handling of each type of packet sent
struct DATAPKT
{
    uint16_t opcode;
    uint16_t blkNumber;
    char data[MAX_DATA_SIZE];
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
    char errMsg[MAX_ERR_SIZE];
}; 
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


//check available port for new connection;
int get_new_port(){
  if(current_port < max_port){
    return current_port++;
  }else{
    fprintf(stderr, "ERROR: Out of available ports\n");
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

int get_request(char * fileName, struct sockaddr_in clientaddr){


}

int put_request(char * fileName, struct sockaddr_in clientaddr){

}

void process_request(void* buffer, struct sockaddr_in clientaddr){
  int req_type = 0;
  struct REQPKT * req_pkt = buffer;

  req_pkt->opcode = ntohs(*((uint16_t*)buffer));
  //GET request
  if(req_pkt->opcode == 1){
    get_request(req_pkt->fileName, clientaddr);
  }else if(req_pkt->opcode = 2){//PUT request
    put_request(req_pkt->fileName, clientaddr);
  }else{//Other request
    fprintf(stderr, "ERROR: Invalid Client request\n");
  }

}


int main(int argc, char ** argv){
  if(argc < 2){
        fprintf(stderr, "ERROR:Invalid number of arguments\n");
        return EXIT_FAILURE;
  }

  int port_start = atoi(argv[1]);
  max_port = atoi(argv[2]);
  current_port = port_start;

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
  servaddr.sin_port = htons(port_start);

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