import csci4220_hw4_pb2
import csci4220_hw4_pb2_grpc

class HashTable(csci4220_hw4_pb2_grpc.KadImplServicer):

    def First(self, request, context):
        print("gottem")
        pass

    '''

    This should return a NodeList object

    @responding_node is a Node object
    @nodes is a list containing 0 or more nodes

    '''
    def FindNode(self, request, context):
        print("harcoded the return values")
        toReturn = csci4220_hw4_pb2.NodeList(responding_node=csci4220_hw4_pb2.Node(id=1, port=1234, address="address"), nodes=[csci4220_hw4_pb2.Node(id=1, port=1234, address="address")])
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