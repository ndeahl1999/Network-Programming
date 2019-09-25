#include "unp.h"


// keep track of the ports
int current_port;
int max_port;


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


// SIGALRM
void sig_alarm(){
  alarm(10);
}


void get_request(int listenfd, struct sockaddr_in * servaddr, char * buffer){
  int tid = ntohs(servaddr->sin_port);
  printf("that port is %d\n", tid);
  char filename[80];
  int block = 0;
  char data[517];
  int current_len;

  unsigned short * opcode_ptr = (unsigned short *)buffer;
  int n;
  //get file
  strcpy(filename, buffer+2);

  // printf("file name is %s\n", filename);
  FILE * f = fopen(filename, "r");

  // couldn't open
  if(f == NULL){ 
    *opcode_ptr = htons(5);
    *(opcode_ptr+1) = htons(1);
    *(buffer+4) = 0;
    //send an error and terminate
    n = sendto(listenfd, buffer, 5, 0, (struct sockaddr*) servaddr, sizeof(*servaddr));
    printf("couldn't find this\n");
   
    return;
  }

  char ltr;
  while(1){
    block++;

    // append to after the op code
    *opcode_ptr = htons(3);
    *(opcode_ptr+1) = htons(block);
     for(n = 4; n < 517; n++){
         if(fscanf(f, "%c", &ltr) == EOF){
          break;
         }
         buffer[n] = ltr;
         //printf("the buffer contains %c\n", *(buffer+n));
     }


    printf("just read the characters with length %d\n", n);
    


    // create the full packet
    for(int i = 0; i < n; i++)
      data[i] = buffer[i];
    current_len = n;


    //send
    printf("Sending message to port %d\n", ntohs(servaddr->sin_port) );

    n = sendto(listenfd, buffer, n, 0,(struct sockaddr*) servaddr, sizeof(*servaddr));

    printf("Sent %d chars to %d\n", n, ntohs(servaddr->sin_port));

    int count = 0;


    socklen_t sockaddr_len;
    // get ack
    while(1){
      printf("now waiting on recv\n");
      n = recvfrom(listenfd, buffer, 517, 0, (struct sockaddr *)&servaddr, &sockaddr_len);
      printf("just finished with recv %d %s\n", n, buffer);
      if(n < 0) {
        if(errno == EINTR) continue;

        printf("got past the errno record\n");

        // wait a second
        if(errno == EWOULDBLOCK){
          if(++count >= 10){
            printf("transaction timed out\n");
            break;
          }
          n = sendto(listenfd, data, current_len,0,(struct sockaddr*) servaddr, sizeof(*servaddr));
        }
        perror("recvfrom");
        printf("this ffailed but not in stderr\n");
        exit(-1);
      }else{
        printf("SOLUTION HERE\n");
        break;
      }
    }


  
    printf("now after getting ack\n");
    printf("now after getting ackasdf\n");
    //check the tid
    printf("%d\n", ntohs(servaddr->sin_port)); //THIS CRASHES IT on the first run through
    printf("got here at leastasd\n");
     if(ntohs(servaddr->sin_port) != tid){
      //  printf("atleast here\n");
       //different client
       *opcode_ptr = htons(5);
       *(opcode_ptr+1) = htons(5);
       *(buffer+4) = 0;
       printf("wrong port\n");
       n = sendto(listenfd, buffer, 5, 0, (struct sockaddr *)servaddr, sizeof(*servaddr));
    }

    printf("just got past checking xtid\n");

    //reset the timout counter
    count = 0;


    // make sure it's not ack
    // that its RRQ
    if(ntohs(*opcode_ptr) != 4) {
      if(ntohs(*opcode_ptr) == 1){

        // reset packet
        // for(int i = 0; i < 517; i++)
        //   buffer[i] = data[i];
        n = sendto(listenfd, data, current_len, 0, (struct sockaddr*) servaddr, sizeof(*servaddr));
      }
    }

    printf("just got done resendnig packet\n");

    // done sending
    if(current_len < 517)
      break;
  }


  *opcode_ptr = htons(4);
  *(opcode_ptr + 1) = htons(block);
  *(buffer+4) = 0;
  n = sendto(listenfd, buffer, 5, 0, (struct sockaddr*) servaddr, sizeof(*servaddr));
  

  fclose(f);
}

void put_reqest(int listenfd, struct sockaddr_in * servaddr, char * buffer){
  int tid = ntohs(servaddr->sin_port);
  char filename[80];
  int block = 0;
  char data[517];
  int last_block = 0;

  //get file
  strcpy(filename, buffer+2);
  FILE * f = fopen(filename, "w");

  unsigned short * opcode_ptr = (unsigned short *)buffer;

  //set the op code 
  *opcode_ptr = htons(4);
  *(opcode_ptr + 1) = htons(block);

  //store contents of this packet
  for(int i = 0; i < 517; i++)
      data[i] = buffer[i];
  int current_len = 4;

  //send the first ack to begin receiving data
  int n = sendto(listenfd, buffer, 4, 0, (struct sockaddr*) servaddr, sizeof(*servaddr));
  int done = 0;

  socklen_t sockaddr_len;
  int count = 0;

  // keep the loop going until all sent
  while(!done){

    // receive data
    n = recvfrom(listenfd, buffer, 517, 0, (struct sockaddr *)&servaddr, &sockaddr_len);
    if(n < 0) {
      if(errno == EINTR) continue;
      //1 second timeout
      if(errno == EWOULDBLOCK){
        if(++count >= 10){
          printf("transaction timed out\n");
          break;
        }
        //restore last packet
        for(int i = 0; i < 517; i++)
          buffer[i] = data[i];
        n = sendto(listenfd, buffer, current_len, 0, (struct sockaddr*) servaddr, sizeof(*servaddr));
        continue;
      }
      perror("recvfrom");
      exit(-1);
    }
    
    // confirm it's from the right TID
    if(ntohs(servaddr->sin_port) != tid){
      //different client
      *opcode_ptr = htons(5);
      *(opcode_ptr+1) = htons(5);
      *(buffer+4) = 0;
      
      n = sendto(listenfd, buffer, 5, 0, (struct sockaddr*) servaddr, sizeof(*servaddr));

      // otherwise try again
      continue;
    }

    //reset the timout counter
    count = 0;

    // make sure its data packet
    if(ntohs(*opcode_ptr) != 3) {
      if(ntohs(*opcode_ptr) == 2){

        // reset packet
        for(int i = 0; i < 517; i++)
          buffer[i] = data[i];
        n = sendto(listenfd, buffer, current_len, 0, (struct sockaddr*) servaddr, sizeof(*servaddr));
      }
      continue;
    }


    // get the block
    block = ntohs(*(opcode_ptr+1));
    if(block == last_block){

      // set data for sending
      for(int i = 0; i < 517; i++)
        buffer[i] = data[i];
      
        n = sendto(listenfd, buffer, current_len, 0, (struct sockaddr*) servaddr, sizeof(*servaddr));
      continue;
    }

    buffer[n] = '\0';
    fprintf(f, "%s\n", buffer+4);

    // done sending
    if(n < 516) 
      done = 1;

    // send ack
    *opcode_ptr = htons(4);
    *(opcode_ptr+1) = htons(block);

    // send packet
    for(int i = 0; i < 517; i++)
      data[i] = buffer[i];
    current_len = 4;


    n = sendto(listenfd, buffer, 4, 0, (struct sockaddr*)servaddr, sizeof(*servaddr));
  }    
  fclose(f);  
}

int main(int argc, char** argv) {
    int n;
    char buffer[517];
    socklen_t sockaddr_len, client_len;
    struct sigaction act;
    

    // interupt sigalarm
    act.sa_handler = sig_alarm;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGALRM, &act, NULL);


    // interupt sigchld
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, NULL);
    
    if(argc < 2){
        fprintf(stderr, "ERROR:Invalid number of arguments\n");
        return EXIT_FAILURE;
    }

    
    int port_start = atoi(argv[1]);
    max_port = atoi(argv[2]);
    
    current_port = port_start;


    int listenfd, connfd;
    pid_t childpid;
    struct sockaddr_in servaddr, clientaddr;

    sockaddr_len = sizeof(servaddr);
    client_len = sizeof(clientaddr);
    // initialize the socket
    if((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ){
      perror("socket failed");
      exit(0);
    }
    memset(&servaddr, 0, sockaddr_len);
    

    servaddr.sin_family      = AF_INET;
	  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	  servaddr.sin_port        = htons(port_start);

    printf("%d\n", port_start);

    // bind to port
    if(bind(listenfd, (struct sockaddr *) &servaddr, sockaddr_len) < 0){
      perror("bind failed");
      exit(0);
    }


    // for later
    unsigned short int opcode;
    unsigned short int * opcode_ptr;


    while(1) {
      memset(&clientaddr, 0, client_len);
      memset (&buffer, 0, sizeof (buffer));
      n = recvfrom(listenfd, buffer, 517, 0, (struct sockaddr *)&clientaddr, &client_len);
      // printf("the port we got stuff from is %d from pid %d\n", ntohs(servaddr.sin_port), getpid());
      if (n < 0)
      {

        // to fix interrupting the system call
        if(errno == EINTR) memset (&buffer, 0, sizeof (buffer));
          continue;
        perror("recvfrom\n");
        exit(-1);
      }
      /* check the opcode */
      opcode_ptr = (unsigned short *)buffer;
      opcode = ntohs(*opcode_ptr);
      if(opcode != 1 && opcode != 2) {
        
        // send error code
        // can't accept non-RRQ or non-WRQ here
        *opcode_ptr = htons(5);
        *(opcode_ptr + 1) = htons(4);
        *(buffer + 4) = 0;
        n = sendto(listenfd, buffer, 5, 0, (struct sockaddr *)&clientaddr, sockaddr_len);
        if(n < 0) {
          perror("sendto");
          exit(-1);
        }
      }
      else {

        // child
        if(fork() == 0) {
          close(listenfd);
          break;
        }
      }
    }

    
  
    clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientaddr.sin_port = htons(++current_port);
    clientaddr.sin_family = AF_INET;

    if((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(-1);
    }

    if(bind(listenfd, (struct sockaddr *)&clientaddr, sockaddr_len) < 0) {
        printf("errored at %d\n", (int)ntohs(clientaddr.sin_port));
        perror("bind error");
        exit(-1);
    }

    struct timeval timeout_interval;
    //set a 1 second timeout for receiving from the socket
    timeout_interval.tv_sec = 1;
    timeout_interval.tv_usec = 0;
    setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, &timeout_interval, sizeof(timeout_interval));

    // if RRQ
    if(opcode == 1)
      get_request(listenfd, &clientaddr, buffer);

    // if WRQ
    if(opcode == 2) 
      put_reqest(listenfd, &servaddr, buffer);

    close(listenfd);
    
    return 0;
}
