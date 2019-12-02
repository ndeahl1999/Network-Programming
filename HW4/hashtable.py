import csci4220_hw4_pb2
import csci4220_hw4_pb2_grpc

class HashTable(csci4220_hw4_pb2_grpc.KadImplServicer):

    k_buckets = {}
    my_port = -1
    my_id = 1
    my_address = ""
    data = {}

    
    def PrintBuckets(self):
        for item in self.k_buckets.items():
            peer_id = ' '.join(item[1])
            print(str(item[0])+": "+peer_id)

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

        
        toReturn = csci4220_hw4_pb2.NodeList(responding_node=csci4220_hw4_pb2.Node(id=self.my_id, port=self.my_port, address=self.my_address), nodes=
        [
            # csci4220_hw4_pb2.Node(id=1, port=1234, address="address")
        ])

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
        return toReturn

    '''



    '''
    def FindValue(self, request, context):
        pass


    '''




    '''
    def Store(self, request, context):
        k = request.key
        v = request.value
        data[k] = v
        for k, v in self.data.items():
            print(str(k) + " " + str(v))
        # pass

    '''



    '''
    def Quit(self, request, context):
        pass