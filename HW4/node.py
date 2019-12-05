'''

Helper class reponsible for saving tuplets of values

address, port, node_id

'''
class Node:
    address = ""
    port = ""
    node_id = ""

    '''
    constructor
    '''
    def __init__(self, a, p, n):
        self.address = a
        self.port = p
        self.node_id = n

    '''
    debug printing str representation
    '''
    def __str__(self):
        return ("Address " + str(self.address) + 
                "\nPort " + str(self.port) +
                "\nID " + str(self.node_id))
