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
            bucket = item[1]
            for index in range(len(bucket)):

                if counter == 0:
                    return
                counter = counter-1

                # send the peer the find node command
                with grpc.insecure_channel(bucket[index].address + ":" + str(bucket[index].port)) as channel:
                    # print("we are sending to port " + str(bucket[index].port))
                    stub = csci4220_hw4_pb2_grpc.KadImplStub(channel)
                    R = stub.FindNode(obj)

                    self.UpdateBucket(item[0],bucket[index])

                    # print(R.nodes)
                    # update k buckets with node (move it to the front)
                    for i in R.nodes:
                        b = self.my_id ^ i.id
                        b = b.bit_length() - 1

                        if (b >= 0):
                            found = False
                            for match in bucket:
                                if match.node_id == i.id:
                                    found = True
                            
                            if (not found):
                                self.UpdateBucket(b, Node(i.address,i.port,i.id))
                                
                            # inserted = self.UpdateBucket(bucket, Node(i.address, i.port, i.id))
                        # if (inserted):
                        #     index = index-1



                    


    '''

    Helper function to create FindValue request to peers

    '''
    def SendFindValue(self, target_key):

        if target_key in self.data.keys():
            print("Found data \""+ self.data[target_key]+ "\" for key " + str(target_key))
            return
        
        obj = csci4220_hw4_pb2.IDKey(node=csci4220_hw4_pb2.Node(id=self.my_id, port=int(self.my_port), address=self.my_address), idkey=target_key)

        # for all peers
        for item in self.k_buckets.items():
            for peer in item[1]:
                with grpc.insecure_channel(peer.address + ":" + str(peer.port)) as channel:

                    # send find value command
                    stub = csci4220_hw4_pb2_grpc.KadImplStub(channel)
                    node = stub.FindValue(obj)
                    
                    # if found
                    if (node.mode_kv == True):
                        print("Found value \"" + node.kv.value + "\" for key " + str(node.kv.key))


    def SendQuit(self):
        obj = csci4220_hw4_pb2.IDKey(node=csci4220_hw4_pb2.Node(id=self.my_id, port=int(self.my_port), address=self.my_address), idkey=self.my_id)

        for item in self.k_buckets.items():
            for peer in item[1]:
                with grpc.insecure_channel(peer.address + ":" + str(peer.port)) as channel:
                    
                    print("Letting "+ str(peer.node_id)+" know I'm quitting.")

                    try:
                        stub = csci4220_hw4_pb2_grpc.KadImplStub(channel)
                        ret = stub.Quit(obj)
                    except:
                        pass


    '''


    bucket_no: int
    new_node: Node

    '''
    def UpdateBucket(self, bucket_no, new_node):
        bucket = self.k_buckets[bucket_no]
        index = -1
        # seeing if the new node is already in the k buckets
        for i in range(len(bucket)):
            if bucket[i].node_id == new_node.node_id:
                # print("Found a bucket to update")
                index = i
        
        # remove it, and add it back to the beginning
        if index != -1:
            del bucket[index]
            bucket.insert(0, new_node)
        # if it's not in there
        else:
            #check size of array
            if len(bucket) >= 2:
                del bucket[1]
            
            bucket.insert(0, new_node)

        # print("contents are ")
        # print(bucket)
        # print("contents are ")
        return

    

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
            

            responding = []

            # responding.append(csci4220_hw4_pb2.Node(id=self.my_id, port=self.my_port, address=self.my_address))

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



        
        return toReturn

    '''

    Should return a KV_Node_Wrapper


    '''
    def FindValue(self, request, context):
        node = request.node # node object (id, port, address)
        key = request.idkey  # int

        print("Serving FindKey("+str(key)+") request for " + str(node.id))
        
        self_node = csci4220_hw4_pb2.Node(id=self.my_id, port=self.my_port, address=self.my_address)
        
        #toReturn = csci4220_hw4_pb2.KV_Node_Wrapper(responding_node=self_node, mode_kv=True, kv=csci4220_hw4_pb2.KeyValue(node=self_node,key=,value=) nodes=[]))
        if key in self.data:
            print("Found")
            return csci4220_hw4_pb2.KV_Node_Wrapper(responding_node=self_node, mode_kv=True, kv=csci4220_hw4_pb2.KeyValue(node=self_node,key=key,value=self.data[key]), nodes=[])
        else:
            return csci4220_hw4_pb2.KV_Node_Wrapper(responding_node=self_node, mode_kv=False, kv=csci4220_hw4_pb2.KeyValue(node=self_node, key=key, value="None"), nodes=[
                # do stuff here in the nodes array
            ])
        



    '''


        The node should send a Store RPC to the single node that has ID closest to the key. Keep in mind that
        the current node may be the closest node and may need to store the key/value pair locally. For simplicty,
        values will never have spaces in them, but may be otherwise-arbitrary strings.
        When calling the Store RPC, the requester should print:
        Storing key <key> at node <remoteID>
        If a node receives a call to Store, it should print:
        Storing key <key> value "<value>"
        (update 12/5): When receiving a store RPC the node receiving the call should locally store the key/value
        pair. It should also update its own k-buckets by adding/updating the requester’s ID to be the most recently
        used

        its basically done
        just need update buckets

        true

    '''
    def Store(self, request, context):
        node = request.node
        k = request.key
        v = request.value
        print("Storing key " + str(k) + " value \"" + str(v) + "\"")
        self.data[k] = v

        toReturn = csci4220_hw4_pb2.IDKey(node=csci4220_hw4_pb2.Node(id=self.my_id, port=self.my_port, address=self.my_address), idkey=self.my_id)
        bucket = self.my_id ^ node.id
        bucket = bucket.bit_length() - 1

        found = False
        # check to make sure that peer exists
        for item in self.k_buckets[bucket]:
            if item.node_id == node.id:
                found = True
        
        # if found, update it
        if found == True:
            # print("Found")
            self.UpdateBucket(bucket, Node(node.address,node.port,node.id))
        # else:
            # print("not found")

    
        return toReturn

    '''
        The node should send a Quit RPC to each node that is in its k-buckets. Before making a call to node
        <remoteID>, the node should print:
        Letting <remoteID> know I'm quitting.
        If a node receives a call to Quit from <remoteID>, and the remote node is in k-bucket , the entry should be
        removed from the k-bucket and the following printed:
        Evicting quitting node <remoteID> from bucket <i>
        Otherwise the node should print the following:
        No record of quitting node <remoteID> in k-buckets.
        Finally, the node should print the following message to stdout, where <ID> is its ID, and then terminate:
        Shut down node <ID>


    '''
    def Quit(self, request, context):
        node = request.node
        node_id = request.idkey

        bucket = -1
        index = -1
        for item in self.k_buckets.items():
            for i in range(len(item[1])):
                if item[1][i].node_id == node_id:
                    bucket = item[0]
                    index = i
        if index != -1:
            print("Evicting quitting node " + str(node_id) + " from bucket " + str(bucket))
            del self.k_buckets[bucket][index]
        else:
            print("No record of quitting node " + str(node_id) + "in k-buckets.")
        
        return request


# print("getting -----")
                    # print(node)
                    # print("getting -----")
                    # found_original = False
                    # for i in node.nodes:
                    #     bucket = self.my_id ^ i.id
                    #     bucket = bucket.bit_length() - 1

                    #     if (i.id == target_id):
                    #         found_original = True

                    #     found = False

                    #     # it's -1 for itself, ignore self
                    #     if bucket >= 0:
                    #         for matches in self.k_buckets[bucket]:
                    #             if matches.node_id == i.id:
                    #                 found=True

                    #         # if not contained in that bucket
                    #         # add it
                    #         # if found == False:
                            
                    #         self.UpdateBucket(bucket,Node(i.address, i.port, i.id))
                    #             # self.k_buckets[bucket].insert(0,Node(i.address, i.port, i.id))
                    # if (found_original):
                    #     return
                    # # self.PrintBuckets()