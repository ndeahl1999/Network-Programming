/* 
    Noah Deahl
    Andrew Qu
    Net Prog HW1
*/

//#include "unp.h"
//
extern "C" {
//#include "unp.h"
#include "../../unpv13e/lib/unp.h"
}
//#include "../../unpv13e/lib/unp.h"

#include "structs.h"




// use later to make sure there doesn't exceed the maximum # of connections
int max_port;

// use to generate new unique port to recvfrom
int current_port;


void handle_write(struct sockaddr_in* sock,char* buffer, int buffer_length);
void handle_read(struct sockaddr_in* sock,char* buffer, int buffer_length);



int send_ack(int listenfd, uint16_t block, struct sockaddr_in * sock, socklen_t sock_len);


int main(int argc, char ** argv)
{

    if(argc < 2){
        fprintf(stderr, "ERROR:Invalid number of arguments\n");
        return EXIT_FAILURE;
    }

    
    int port_start = atoi(argv[1]);
    max_port = atoi(argv[2]);
    
    current_port = port_start;

    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in servaddr;

    // initialize the socket
    if((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
      perror("socket failed");
      exit(0);
    }

    bzero(&servaddr, sizeof(servaddr));
	  servaddr.sin_family      = AF_INET;
	  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	  servaddr.sin_port        = htons(port_start);

    // bind to port
    if(bind(listenfd, (SA *) &servaddr, sizeof(servaddr)) < 0){
      perror("bind failed");
      exit(0);
    }
    
    // dispaly the address listening to currently
    printf("%d\n", ntohs(servaddr.sin_port));


    // start to receive
    int msg_len;
    char buffer[1024];
    unsigned short int opcode;
    unsigned short int * opcode_ptr;

    for( ; ; ){

      struct sockaddr_in client_socket;
      socklen_t slen = sizeof(client_socket);

      //get a message
      msg_len = recvfrom(listenfd, &buffer, sizeof(buffer), 0, (struct sockaddr *)&client_socket, &slen);
      if(msg_len < 0){
        if(errno == EINTR)
          continue;
        perror("recvfrom error");
        exit(0);
      }


      // got message successfully
      // get the type of message
      opcode_ptr = (unsigned short int *) buffer;
      opcode = ntohs(*opcode_ptr);
      printf("the opcode message is : %d\n", opcode);

      if(opcode != 1 && opcode != 2){
      
        *opcode_ptr = htons(5);
        *(opcode_ptr+1) = htons(4);
        msg_len = sendto(listenfd, &buffer, 5, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

        if(msg_len < 0){
          perror("failed to send");
          return 1;
        }
      
      }
      else{
        if(fork() == 0){

          close(listenfd);

          if(opcode == 1)
            handle_read(&client_socket, buffer, 1024);
          if(opcode == 2)
            handle_write(&client_socket, buffer, 1024);
          break;
        
        }
      }
    }
    return 0;

}



void handle_write(struct sockaddr_in* sock, char* buffer, int buffer_length){




}


void handle_read(struct sockaddr_in* sock, char* buffer, int buffer_length){

  char *file_name = strchr(buffer, '\0')+1;
  printf("file name is [%s]\n", file_name);

  char *mode= strchr(file_name, '\0')+1;
  printf(" mode is %s\n", mode);

  if(strcmp(mode, "octet") !=0){
  
    perror("not octet");
    return;
  }



  struct timeval tv;
  tv.tv_sec = 1 ;
  tv.tv_usec = 0;
  //TFTP uses UDP so let's check for protocol
  
  //create socket 
  
  int listenfd;
  if((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
  
    perror("socket failed");
    return;
  }

  if(setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) <0){
  
  
    perror("failed to set sock");
    return;
  }

  FILE *fd;
  fd = fopen(file_name, "r");
  if(fd == NULL){
  
    perror("file couldn't");
  
    return;

  }


  int block = 0;
  int current_len = 0;

  bool done = false;

  while(!done){
  
    char data[512];
    current_len = fread(data, 1,sizeof(data), fd);
    data[current_len] = '\0';
    block++;

    if(current_len < 512)
      done = true;

    int count = 10;
    
    data_packet message;
    
    
    while(count > 0){

      message.op_code = htons(3);
      message.block = htons(block);
      memcpy(message.data, data, current_len);
      printf("data read: %s\n", data);



      
    }
  }






}



// dedicated function to sending acknowledgement 
int send_ack(int listenfd, uint16_t block, struct sockaddr_in * sock, socklen_t sock_len){

  ack_packet packet;

  packet.op_code= htons(4);
  packet.block = htons(block);

  ssize_t bytes_sent;
  if((bytes_sent = sendto(listenfd, &packet, sizeof(packet), 0, (struct sockaddr *) sock, sock_len)) < 0 ){
    perror("sendto failed");
    exit(0);
  }

  return bytes_sent;

}



