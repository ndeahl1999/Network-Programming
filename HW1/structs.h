/*  Request Packet has format:
    
      2 bytes     string    1 byte   string   1 byte
     ------------------------------------------------   
    | Opcode |  Filename  |   0  |    Mode    |   0  |
     ------------------------------------------------
*/
typedef struct{
    uint16_t op_code;
    char filename[1024];
    // no need for mode, only binary
}request_packet;

/*  DATA Packet has format:
    
     2 bytes     2 bytes      n bytes
     ----------------------------------
    | Opcode |   Block #  |   Data     |
     ----------------------------------
*/
typedef struct{
    uint16_t op_code;
    uint16_t block;
    char data[1024]; 
    
}data_packet;

/*  ACK Packet has format:
    
      2 bytes     2 bytes
     ---------------------
    | Opcode |   Block #  |
     ---------------------
*/
typedef struct{
    uint16_t op_code;
    uint16_t block;
}ack_packet;

/*  ERROR Packet has format:
    
      2 bytes     2 bytes      string    1 byte
     -----------------------------------------
    | Opcode |  ErrorCode |   ErrMsg   |   0  |
     -----------------------------------------
*/
typedef struct{
    uint16_t opcode;
    char error[1024];
    uint16_t code; 
    
}error_packet;

