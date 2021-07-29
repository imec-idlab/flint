from .Agent import Agent


# Template for future adapters, also see BasicMqtt-/-HttpAdapter for examples

class MyAgent(Agent):
    
    # --------------------- MANDATORY ---------------------- #

    # Always override the init method with this
    def __init__(self, *args):
        super().__init__(*args)


    def connect(self):
        #set up a connection with downlink server. 
        #call on_connect when connection works (by callback or do a direct call in this method)
        return

    def disconnect(self, *args):
        #disconnect from downlink server. 
        #call on_disconnect when finished
        return
    
    def receive_from_downlink(self, message, *args):
        # make sure the adapter client calls this method when a packet is received
        # perform any necessary preprocessing like converting str to dict, call self.from_downlink_to_central(message) on the end to pass the data through the other adapter methods
        self.from_downlink_to_central(processedMessage)
        return
    
    def send_downlink(self, message, *args)
        # send the packet to the downlink server, use self.connection_ip en self.port_down
        return

    # --------------------- OPTIONAL ---------------------- #
        
    
    def initiate_adapter(self, config):
        #set important data before attempting a connection (use the config object, use self.connection_ip and self.port_uplink)
        #eg create client object ect.
        
        # make sure receive_from_downlink is called when a packet from the downlink server arrives
        # client.on_message = 
        return

    # -------- Template method hooks 

    def validate_downlink_packet(self, message, *args):
        valid = ... # (bool)
        #validate a message received from downlink
        return valid

    def validate_central_packet(self, message, *args):
        valid = ... # (bool)
        # validate a message that came from AdapterBinding / the other adapter
        return valid

    def transform_downlink_to_central(self, message, *args):
        newMessage = {
            'field1' : message['param']
        }
        # after validation, transform a received downlink message to send it to the other adapter
        return message

    def transform_central_to_downlink(self, message, *args):
        newMessage = {
            'field1' : message['param']
        }
        # after validation, transform a received message from AdapterBinding / the other adapter and convert it to the message to send downlink
        return message

    # Extra comments

    # The following (final) template methods are copied from the abstract Adapter class, it should help knowing when the template method hooks are called

    # def from_downlink_to_central(self, message):
    #    if self.validate_downlink_packet(message):
    #        msg = self.transform_downlink_to_central(message)
    #        self.adapterbinding.process_message(msg, self.central_adapter_flag)

    # def from_central_to_downlink(self, message):
    #    if self.validate_central_packet(message):
    #        msg = self.transform_central_to_downlink(message)
    #        if self.connected:
    #            self.send_downlink(msg)