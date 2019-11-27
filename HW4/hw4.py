#!/usr/bin/env python3
from concurrent import futures
import sys  # For sys.argv, sys.exit()
import socket  # for gethostbyname()

import grpc

import csci4220_hw4_pb2
import csci4220_hw4_pb2_grpc



class HashTable(csci4220_hw4_pb2_grpc.KadImplServicer):

    def First(self, request, context):
        print("gottem")
        pass

def run():
    if len(sys.argv) != 4:
        print("Error, correct usage is {} [my id] [my port] [k]".format(sys.argv[0]))
        sys.exit(-1)
    local_id = int(sys.argv[1])
    my_port = str(int(sys.argv[2])) # add_insecure_port() will want a string
    k = int(sys.argv[3])
    my_hostname = socket.gethostname() # Gets my host name
    my_address = socket.gethostbyname(my_hostname) # Gets my IP address from my hostname
    ''' Use the following code to convert a hostname to an IP and start a channel
    Note that every stub needs a channel attached to it
    When you are done with a channel you should call .close() on the channel.
    Submitty may kill your program if you have too many file descriptors open
    at the same time.'''

    # server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    # csci4220_hw4_pb2_grpc.add_KadImplServicer_to_server(HashTable(), server)
    # server.add_insecure_port('[::]:50051')
    # server.start()
    # server.wait_for_termination()
    # remote_addr = socket.gethostbyname(remote_addr_string)
    # remote_port = int(remote_port_string)
    # channel = grpc.insecure_channel(remote_addr + ':' + str(remote_port))

    buckets = pow(2, k)
    k_buckets = [None] * 4
    print(k_buckets)

    while (True):
        stdin = str(input())
        arguments = stdin.split(" ")
        if (arguments[0] == "BOOTSTRAP"):
            print("handle bootstrapping here")
        if (arguments[0] == "STORE"):
            print("handle store here")
        if (arguments[0] == "FIND_VALUE"):
            print("handle find_value here")
        if (arguments[0] == "QUIT"):
            print("handle quit here")

    

if __name__ == '__main__':
    run()
