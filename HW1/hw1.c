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


unsigned short port_start;
unsigned short port_end;
unsigned short current_port;

unsigned short find_port(){
    current_port++;
    if(current_port > port_end){
        fprintf(stderr, "ERROR: Ports outside of given range\n");
        return EXIT_FAILURE;
    }else
    {
        return current_port;
    }
    
}

int main(int argc, char ** argv)
{

    if(argc < 2){
        
        fprintf(stderr, "ERROR:Invalid number of arguments\n");
        return EXIT_FAILURE;
    }

    port_start = htons(atoi(argv[1]));
    port_end = htons(atoi(argv[2]));
    current_port = port_start;

    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in clientaddr, servaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
	  servaddr.sin_family      = AF_INET;
	  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	  servaddr.sin_port        = htons(port_start);

    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

    for( ; ; ){
        clilen = sizeof(clientaddr);
        if ( (connfd = accept(listenfd, (SA *) &clientaddr, &clilen)) < 0) {
			if (errno == EINTR)
				continue;		/* back to for() */
			else
				err_sys("accept error");
		}


		if ( (childpid = Fork()) == 0) {	/* child process */
			Close(listenfd);	/* close listening socket */
			str_echo(connfd);	/* process the request */
			exit(0);
		}
		Close(connfd);			/* parent closes connected socket */

    }

}
