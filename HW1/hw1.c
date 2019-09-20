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

/*  Request Packet has format:
    
      2 bytes     string    1 byte   string   1 byte
     ------------------------------------------------   
    | Opcode |  Filename  |   0  |    Mode    |   0  |
     ------------------------------------------------
*/
typedef struct{
    char * opcode;
    char * filename;
    char * mode; 
    
}request_packet;

/*  DATA Packet has format:
    
     2 bytes     2 bytes      n bytes
     ----------------------------------
    | Opcode |   Block #  |   Data     |
     ----------------------------------
*/
typedef struct{
    char * opcode;
    char * block;
    char * data; 
    
}data_packet;

/*  ACK Packet has format:
    
      2 bytes     2 bytes
     ---------------------
    | Opcode |   Block #  |
     ---------------------
*/
typedef struct{
    char * opcode;
    char * block;
}ack_packet;

/*  ERROR Packet has format:
    
      2 bytes     2 bytes      string    1 byte
     -----------------------------------------
    | Opcode |  ErrorCode |   ErrMsg   |   0  |
     -----------------------------------------
*/
typedef struct{
    char * opcode;
    char * block;
    char * data; 
    
}error_packet;

int port_start;
int port_end;
int current_port;


int main(int argc, char ** argv)
{

    if(argc < 2){
        fprintf(stderr, "ERROR:Invalid number of arguments\n");
        return EXIT_FAILURE;
    }

    port_start = atoi(argv[1]);
    port_end = atoi(argv[2]);
    current_port = port_start;

    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in clientaddr, servaddr;

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
      unsigned short int *op_code_network = (unsigned short *) buffer;
      unsigned short int op_code = ntohs(*opcode_ptr);
      printf("the opcode message is : %d\n", opcode);

      // GET request (serv -> client)
      if(op_code == 1){
        printf("this is the get request\n");

      }
      // PUT request (client -> serv)
      else if(op_code == 2){
        printf("this is the put request\n");

      }
      // DATA packet
      else if(op_code == 3){
        printf("this is a data packet\n");

      }
      // ACK Packet
      else if(op_code == 4){
        printf("this is an ACK packet\n");

      }
      else{
        printf("got error\n");
      }



      

      
    }

    return 0;

}
