import csci4220_hw4_pb2
import csci4220_hw4_pb2_grpc
import grpc
from node import *
class HashTable(csci4220_hw4_pb2_grpc.KadImplServicer):

    peers=[]
    k_buckets = {}
    my_port = -1
    my_id = 1
    my_address = ""
    data = {}

    
    def PrintBuckets(self):
        for item in self.k_buckets.items():
            peers = ""
            for peer in item[1]:
                peers = peers + " "
                peers = peers + str(peer.node_id) + ":" + str(peer.port)
            print(str(item[0]) + ":" + peers)

    def SendFindNode(self, target_id):
        obj = csci4220_hw4_pb2.IDKey(node=csci4220_hw4_pb2.Node(id=self.my_id, port=int(self.my_port), address=self.my_address),
        idkey=target_id)
        counter = 2
        for item in self.k_buckets.items():
            for peer in item[1]:
                if counter == 0:
                    return
                counter = counter-1

                # send the peer the find node command
                with grpc.insecure_channel(peer.address + ":" + str(peer.port)) as channel:
                    stub = csci4220_hw4_pb2_grpc.KadImplStub(channel)
                    node = stub.FindNode(obj)
                    
                    for i in node.nodes:
                        bucket = self.my_id ^ i.id
                        bucket = bucket.bit_length() - 1

                        found = False

                        # it's -1 for itself, ignore self
                        if bucket >= 0:
                            for matches in self.k_buckets[bucket]:
                                if matches.node_id == i.id:
                                    found=True

                            # if not contained in that bucket
                            # add it
                            if found == False:
                                self.k_buckets[bucket].insert(0,Node(i.address, i.port, i.id))


    def SendFindValue(self, target_key):
        print("doing find value")
        obj = csci4220_hw4_pb2.IDKey(node=csci4220_hw4_pb2.Node(id=self.my_id, port=int(self.my_port), address=self.my_address), idkey=target_key)
        for item in self.k_buckets.items():
            for peer in item[1]:
                with grpc.insecure_channel(peer.address + ":" + str(peer.port)) as channel:
                    stub = csci4220_hw4_pb2_grpc.KadImplStub(channel)
                    node = stub.FindValue(obj)

                    print("the results are vvvvvvvvvv")
                    print(node)
    def sendQuit(self):
        pass

    '''
    This should return a NodeList object

    params:
    @request is the contents of the arguments in the call
      Node object
        id: int
        port: int
        address: string

    NodeList
    @responding_node is a Node object
    @nodes is a list containing 0 or more nodes

    '''
    def FindNode(self, request, context):
        # print(request.node)
        to_add = request.node
        request_id = request.idkey
        print("Serving FindNode("+str(to_add.id) + ") request for " + str(request_id))

        # handle bootstrap call
        if (to_add.id == request_id):

            # get the bucket
            bucket = self.my_id ^ to_add.id
            bucket = bucket.bit_length() - 1

            n = Node(to_add.address, to_add.port, to_add.id)
            self.k_buckets[bucket].insert(0, n)

            toReturn = csci4220_hw4_pb2.NodeList(responding_node=csci4220_hw4_pb2.Node(id=self.my_id, port=self.my_port, address=self.my_address), nodes=
            [
                # csci4220_hw4_pb2.Node(id=1, port=1234, address="address")
            ])

            return toReturn
        else:
            
            # obj = csci4220_hw4_pb2.IDKey(node=csci4220_hw4_pb2.Node(id=self.my_id, port=int(self.my_port), address="localhost"),
            # idkey=request_id)

            responding = []

            responding.append(csci4220_hw4_pb2.Node(id=self.my_id, port=self.my_port, address=self.my_address))

            # for all buckets
            for item in self.k_buckets.items():

                # for all peers
                for peer in item[1]:
                    responding.append(csci4220_hw4_pb2.Node(id=peer.node_id, port=peer.port, address=peer.address))


            # create the return object
            # using the array as the nodelist
            toReturn = csci4220_hw4_pb2.NodeList(responding_node=csci4220_hw4_pb2.Node(id=self.my_id, port=self.my_port, address=self.my_address), nodes=
            responding)
            # print(responding)


                    

                    # connect to the channel
                    # with grpc.insecure_channel(peer.address + ":" + str(peer.port)) as channel:
                    #     stub = csci4220_hw4_pb2_grpc.KadImplStub(channel)

                    #     # node = stub.FindNode(obj)
                    #     print("making requests to all not already asked peers here")
                        # print(node)


            return toReturn

        

        
        

        '''
         while some of the k closest have not been asked
           S = the k closest ID to <nodeID>
           S' = nodes in S not yet contacted
           for node in S':
               R = node.FindNode(<nodeID>)

               update k-buckets with node

               if node in R was already in a k-bucket, position does not chnage
               if it was not in the bucket yet, added as most recently used in that bucket
               may kick out node from above

               update k-buckets with all nodes in R

            if <nodeID> has been found, stop
           
        '''

        visited = []
        # while 

        # for k, v in self.k_buckets.items():


        
        return toReturn

    '''



    '''
    def FindValue(self, request, context):
        node = request.node # node object (id, port, address)
        key = request.idkey  # int
        
        self_node = csci4220_hw4_pb2.Node(id=self.my_id, port=self.my_port, address=self.my_address)
        
        #toReturn = csci4220_hw4_pb2.KV_Node_Wrapper(responding_node=self_node, mode_kv=True, kv=csci4220_hw4_pb2.KeyValue(node=self_node,key=,value=) nodes=[]))
        if key in self.data:
            return csci4220_hw4_pb2.KV_Node_Wrapper(responding_node=self_node, mode_kv=True, kv=csci4220_hw4_pb2.KeyValue(node=self_node,key=key,value=self.data[key]), nodes=[])
        else:
            return csci4220_hw4_pb2.KV_Node_Wrapper(responding_node=self_node, mode_kv=False, kv=csci4220_hw4_pb2.KeyValue(node=self_node, key=key, value="None"), nodes=[
                # do stuff here in the nodes array
            ])
        



    '''




    '''
    def Store(self, request, context):
        k = request.key
        v = request.value
        print("Storing key " + str(k) + " value \"" + str(v) + "\"")
        self.data[k] = v

        toReturn = csci4220_hw4_pb2.IDKey(node=csci4220_hw4_pb2.Node(id=self.my_id, port=self.my_port, address=self.my_address), idkey=self.my_id)

        return toReturn
        # for k, v in self.data.items():
        #     print(str(k) + " " + str(v))

        # pass

    '''



    '''
    def Quit(self, request, context):
        pass