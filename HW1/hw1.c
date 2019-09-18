/* 
    Noah Deahl
    Andrew Qu
    Net Prog HW1
*/

#include "unp.h"

int main(int argc, char ** argv)
{

    if(argc < 2){
        perror("ERROR: Invalid arguments");
        return EXIT_FAILURE;
    }

    unsigned short port_start = htons(atoi(argv[1]));
    unsigned short port_end = htons(atoi(argv[2]));

    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in client, server;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

    for( ; ; ){
        clilen = sizeof(client);
        if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
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