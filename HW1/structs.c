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

