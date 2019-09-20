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






// use later to make sure there doesn't exceed the maximum # of connections
int max_port;

// use to generate new unique port to recvfrom
int current_port;



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
      // get the type of message
      unsigned short int *op_code_network = (unsigned short *) buffer;
      unsigned short int op_code = ntohs(*op_code_network);
      printf("the opcode message is : %d\n", op_code);

      //TODO:
      // handle each of these cases in a separate method instead of just in
      // the main


      // GET request (serv -> client)
      if(op_code == 1){
        printf("this is the get request\n");
        if(fork() == 0){
          close(listenfd);
          break;
        }

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

    current_port++;

    // creating the write listener
    //TODO:
    //MOVE THIS INTO A SEPARATE FUNCTION SO IT'S NOT SO MESSY
    //HANDLE READ AND WRITE IN SEPARATE FUNCTION
    struct sockaddr_in sock_info;
    int sockaddr_len = sizeof(sock_info);
    struct timeval timeout_interval;


    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(current_port);
    servaddr.sin_family = AF_INET;

    if((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
      perror(" socket failed ");
      exit(0);
    }

    if(bind(listenfd, (struct sockaddr *)&sock_info, sockaddr_len) < 0){
      perror("bind faield");
      exit(0);
    }
    timeout_interval.tv_sec = 1;
    timeout_interval.tv_usec = 0;
    setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, &timeout_interval, sizeof(timeout_interval));

    




    return 0;

}
