import csci4220_hw4_pb2
import csci4220_hw4_pb2_grpc

class HashTable(csci4220_hw4_pb2_grpc.KadImplServicer):

    k_buckets = {}
    my_port = -1
    my_id = 1
    my_address = ""

    
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
        print("harcoded the return values")
        # print(request.node)
        to_add = request.node

        
        toReturn = csci4220_hw4_pb2.NodeList(responding_node=csci4220_hw4_pb2.Node(id=self.my_id, port=self.my_port, address=self.my_address), nodes=
        [
            # csci4220_hw4_pb2.Node(id=1, port=1234, address="address")
        ])
        return toReturn

    '''



    '''
    def FindValue(self, request, context):
        pass


    '''




    '''
    def Store(self, request, context):
        pass

    '''



    '''
    def Quit(self, request, context):
        pass