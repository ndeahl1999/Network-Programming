import csci4220_hw4_pb2
import csci4220_hw4_pb2_grpc
import grpc
from node import *

'''

Main representation of the server

'''
class HashTable(csci4220_hw4_pb2_grpc.KadImplServicer):

    # the peers and the buckets store
    k_buckets = {}

    # data about self
    my_port = -1
    my_id = 1
    my_address = ""

    # the key value store
    data = {}


    '''

    Helper function to print bucket # and contents

    '''    
    def PrintBuckets(self):
        for item in self.k_buckets.items():
            peers = ""
            for peer in item[1]:
                peers = peers + " "
                peers = peers + str(peer.node_id) + ":" + str(peer.port)
            print(str(item[0]) + ":" + peers)

    '''

    Helper function to send Bootstrap request to peer

    '''
    def SendBootstrap(self, peer_host, peer_port):
        # temporarily connect with them to get 
        with grpc.insecure_channel(peer_host + ":" + peer_port) as channel:
            
            # access the remote server
            stub = csci4220_hw4_pb2_grpc.KadImplStub(channel)
            obj = csci4220_hw4_pb2.IDKey(node=csci4220_hw4_pb2.Node(id=self.my_id, port=int(self.my_port), address=self.my_address), idkey=self.my_id)
            
            # node contains the return from FindNode
            node = stub.FindNode(obj)

            # get the bucket it should be stored in
            bucket = self.my_id ^ node.responding_node.id
            bucket = bucket.bit_length() - 1
            

            # create a new node from the responding node and append it
            n = Node(node.responding_node.address, node.responding_node.port, node.responding_node.id)
            self.UpdateBucket(bucket,n)

            # add all the nodes that were neighbors
            for i in node.nodes:
                b = self.my_id ^ i.id
                b = b.bit_length() - 1

                if (b >= 0):
                    self.UpdateBucket(b, Node(i.address, i.port, i.id))
            print("After BOOTSTRAP(" + str(node.responding_node.id) + "), k_buckets now look like:")
            self.PrintBuckets()

    '''

    Helper function to send FindNode request to peers

    Takes in parameter of target_id trying to find

    '''
    def SendFindNode(self, target_id):
        obj = csci4220_hw4_pb2.IDKey(node=csci4220_hw4_pb2.Node(id=self.my_id, port=int(self.my_port), address=self.my_address),
        idkey=target_id)

        # keep track of visited
        visited = []
        for item in self.k_buckets.items():
            bucket = item[1]
            for index in range(len(bucket)):

                # if already visited
                if (bucket[index].node_id in visited):
                    print("alerady visited")
                    continue

                # send the peer the find node command
                with grpc.insecure_channel(bucket[index].address + ":" + str(bucket[index].port)) as channel:
                    stub = csci4220_hw4_pb2_grpc.KadImplStub(channel)

                    # make the request
                    R = stub.FindNode(obj)

                    # add this to visited
                    visited.append(bucket[index].node_id)

                    # update the position of this first node
                    self.UpdateBucket(item[0],bucket[index])

                    # update k buckets with node (move it to the front)
                    for i in R.nodes:
                        b = self.my_id ^ i.id
                        b = b.bit_length() - 1

                        # make sure not self
                        if (b >= 0):
                            found = False
                            for match in bucket:
                                if match.node_id == i.id:
                                    found = True

                            # found it
                            if (target_id == i.id):
                                print("Found destination id " + str(target_id))
                            self.UpdateBucket(b, Node(i.address, i.port, i.id))
    '''

    Helper function to create FindValue request to peers

    '''
    def SendFindValue(self, target_key):

        if target_key in self.data.keys():
            print("Found data \""+ self.data[target_key]+ "\" for key " + str(target_key))
            return
        
        obj = csci4220_hw4_pb2.IDKey(node=csci4220_hw4_pb2.Node(id=self.my_id, port=int(self.my_port), address=self.my_address), idkey=target_key)

        visited = []
        visited.append(self.my_id)

        # for all peers
        to_sort = []
        for item in self.k_buckets.items():
            for peer in item[1]:
                to_sort.append(peer)
        
        # sort the peers based on distance to target_key
        to_sort.sort(key=lambda node: (target_key ^ node.node_id).bit_length() -1)

        # for all peers in order
        for peer in to_sort:
            with grpc.insecure_channel(peer.address + ":" + str(peer.port)) as channel:

                # send find value command
                stub = csci4220_hw4_pb2_grpc.KadImplStub(channel)
                R = stub.FindValue(obj)
                visited.append(peer.node_id)

                b = peer.node_id ^ self.my_id
                b = b.bit_length() - 1

                # update location of the peer
                self.UpdateBucket(b, peer)
                
                # if found
                if (R.mode_kv == True):
                    print("Found value \"" + R.kv.value + "\" for key " + str(R.kv.key))
                    return
                else:
                    for i in R.nodes:
                        if i.id not in visited:

                            b = self.my_id ^ i.id
                            b = b.bit_length() - 1

                            self.UpdateBucket(b, Node(i.address, i.port, i.id))

                            # search through the returned k nearest peers
                            with grpc.insecure_channel(i.address + ":" + str(i.port)) as to_ask:
                                stub = csci4220_hw4_pb2_grpc.KadImplStub(to_ask)
                                response = stub.FindValue(obj)
                                visited.append(i.id)

                                # if found in peer list
                                if (response.mode_kv == True):
                                    print("Found value \"" + response.kv.value + "\" for key " + str(response.kv.key))
                                    return
        # if haven't found at this point, print that it couldn't be found
        print("Could not find key " + str(target_key))
        


    '''

    Helper function to send a Quit request to all peers

    '''
    def SendQuit(self):
        obj = csci4220_hw4_pb2.IDKey(node=csci4220_hw4_pb2.Node(id=self.my_id, port=int(self.my_port), address=self.my_address), idkey=self.my_id)

        # for all peers
        for item in self.k_buckets.items():
            for peer in item[1]:
                #send a quit to all pears
                with grpc.insecure_channel(peer.address + ":" + str(peer.port)) as channel:
                    print("Letting "+ str(peer.node_id)+" know I'm quitting.")
                    
                    # send quit to them
                    try:
                        stub = csci4220_hw4_pb2_grpc.KadImplStub(channel)
                        ret = stub.Quit(obj)
                    # if quit does not connect, do nothing
                    # hacky way as of submitty 195
                    except:
                        pass


    '''

    Helper function responsible for keeping the size of each
        bucket to 2, and also for appending if it already exists

    bucket_no: int
    new_node: Node

    '''
    def UpdateBucket(self, bucket_no, new_node):
        bucket = self.k_buckets[bucket_no]
        index = -1
        # seeing if the new node is already in the k buckets
        for i in range(len(bucket)):
            if bucket[i].node_id == new_node.node_id:
                index = i
        
        # remove it, and add it back to the beginning
        if index != -1:
            del bucket[index]
            bucket.append(new_node)
        # if it's not in there
        else:
            #check size of array
            if len(bucket) >= 2:
                del bucket[0]
            bucket.append(new_node)

    

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
        to_add = request.node
        request_id = request.idkey
        print("Serving FindNode("+str(to_add.id) + ") request for " + str(request_id))

        # handle bootstrap call
        if (to_add.id == request_id):

            # get the bucket
            bucket = self.my_id ^ to_add.id
            bucket = bucket.bit_length() - 1

            n = Node(to_add.address, to_add.port, to_add.id)
            self.UpdateBucket(bucket, n)
            responding = []
            for item in self.k_buckets.items():
                for peer in item[1]:
                    responding.append(csci4220_hw4_pb2.Node(id=peer.node_id, port=peer.port, address=peer.address))
            
            # create the node list
            toReturn = csci4220_hw4_pb2.NodeList(responding_node=csci4220_hw4_pb2.Node(id=self.my_id, port=self.my_port, address=self.my_address), nodes=responding)

            return toReturn

        else:
            
            responding = []
            # for all buckets
            for item in self.k_buckets.items():

                # for all peers
                for peer in item[1]:
                    responding.append(csci4220_hw4_pb2.Node(id=peer.node_id, port=peer.port, address=peer.address))

            # create the return object
            # using the array as the nodelist
            toReturn = csci4220_hw4_pb2.NodeList(responding_node=csci4220_hw4_pb2.Node(id=self.my_id, port=self.my_port, address=self.my_address), nodes=
            responding)

            return toReturn

        return toReturn

    '''

    Should return a KV_Node_Wrapper


    '''
    def FindValue(self, request, context):
        node = request.node # node object (id, port, address)
        key = request.idkey  # int

        print("Serving FindKey(" + str(key) + ") request for " + str(node.id))

        # create self node to attach to findValue call
        self_node = csci4220_hw4_pb2.Node(id=self.my_id, port=self.my_port, address=self.my_address)

        
        # update the bucket so requesting is the most recent one
        b = self.my_id ^ node.id
        b = b.bit_length() - 1

        self.UpdateBucket(b, Node(node.address, node.port, node.id))

        # if self contains key
        if key in self.data:
            return csci4220_hw4_pb2.KV_Node_Wrapper(responding_node=self_node, mode_kv=True, kv=csci4220_hw4_pb2.KeyValue(node=self_node,key=key,value=self.data[key]), nodes=[])
        # if self doesn't contain key, ask peers 
        else:

            # create the nodes list to return
            nodes_list = []
            for item in self.k_buckets.items():
                for peer in item[1]:
                    nodes_list.append(csci4220_hw4_pb2.Node(id=peer.node_id, port=peer.port, address=peer.address))

            return csci4220_hw4_pb2.KV_Node_Wrapper(responding_node=self_node, mode_kv=False, kv=csci4220_hw4_pb2.KeyValue(node=self_node, key=key, value="None"), nodes=nodes_list)
        
    '''

    responsible for store RPC call

    returns IDKey with the requested's info

    '''
    def Store(self, request, context):
        node = request.node
        k = request.key
        v = request.value

        # received request, simply add it to the key value store
        print("Storing key " + str(k) + " value \"" + str(v) + "\"")
        self.data[k] = v

        # create the return object
        toReturn = csci4220_hw4_pb2.IDKey(node=csci4220_hw4_pb2.Node(id=self.my_id, port=self.my_port, address=self.my_address), idkey=self.my_id)


        # update the location of the sending store command
        bucket = self.my_id ^ node.id
        bucket = bucket.bit_length() - 1

        found = False
        # check to make sure that peer exists
        for item in self.k_buckets[bucket]:
            if item.node_id == node.id:
                found = True
        
        # if found, update it
        if found == True:
            self.UpdateBucket(bucket, Node(node.address,node.port,node.id))
    
        return toReturn

    '''
        
    Quit RPC call

    returns IDKey becuase something needs to be returned

    evicts the quitting node

    '''
    def Quit(self, request, context):
        node = request.node
        node_id = request.idkey

        # variables to track where the node is
        bucket = -1
        index = -1

        # check to see if the quitting node is a neighbor
        for item in self.k_buckets.items():
            for i in range(len(item[1])):
                if item[1][i].node_id == node_id:
                    bucket = item[0]
                    index = i
        # remove it from buckets
        if index != -1:
            print("Evicting quitting node " + str(node_id) + " from bucket " + str(bucket))
            del self.k_buckets[bucket][index]

        # not a peer
        else:
            print("No record of quitting node " + str(node_id) + " in k-buckets.")
        
        return request

