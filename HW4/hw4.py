#!/usr/bin/env python3
from concurrent import futures
import sys  # For sys.argv, sys.exit()
import socket  # for gethostbyname()

import grpc

import csci4220_hw4_pb2
import csci4220_hw4_pb2_grpc


'''
The file with the implementation for the homework server
Split into a separate file for readability
'''
from hashtable import *

from node import *



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

    # server.wait_for_termination()
    # channel = grpc.insecure_channel(remote_addr + ':' + str(remote_port))


    # start the server in the background

    
    hash_table = HashTable()
    hash_table.my_port = int(my_port)
    hash_table.my_id = local_id
    hash_table.my_address = my_address

    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    csci4220_hw4_pb2_grpc.add_KadImplServicer_to_server(hash_table, server)
    server.add_insecure_port('[::]:'+my_port)
    server.start()

    # initializing the buckets 2^k
    buckets = pow(2, k)
    for i in range(buckets):
        hash_table.k_buckets[i] = []
    

    # main listening loop
    while (True):
        stdin = str(input())
        arguments = stdin.split(" ")
        # connect to another peer
        if (arguments[0] == "BOOTSTRAP"):
            peer_host = arguments[1]
            peer_port = arguments[2]

            # temporarily connect with them to get 
            with grpc.insecure_channel(peer_host + ":" + peer_port) as channel:
                
                # access the remote server
                stub = csci4220_hw4_pb2_grpc.KadImplStub(channel)
                obj = csci4220_hw4_pb2.IDKey(node=csci4220_hw4_pb2.Node(id=local_id, port=int(my_port), address="localhost"), idkey=local_id)
                
                # node contains the return from FindNode
                node = stub.FindNode(obj)

                # get the bucket it should be stored in
                bucket = hash_table.my_id ^ node.responding_node.id
                bucket = bucket.bit_length() - 1
                


                n = Node(node.responding_node.address, node.responding_node.port, node.responding_node.id)
                # hash_table.peers.append(n)
                hash_table.k_buckets[bucket].insert(0,n)
                # bootstrapped = node.responding_node
                

            
            print("After BOOTSTRAP(" + str(node.responding_node.id) + "), k_buckets now look like:")
            hash_table.PrintBuckets()

            # for i in range(len(hash_table.peers)):
            #     print(hash_table.peers[i])
                # print(node.responding_node.port)

        if (arguments[0] == "STORE"):
            print("handle store here")
        if (arguments[0] == "FIND_NODE"):
            target_id = int(arguments[1])
            




        if (arguments[0] == "FIND_VALUE"):
            print("handle find_value here")
        if (arguments[0] == "QUIT"):
            print("handle quit here")


    

if __name__ == '__main__':
    run()
