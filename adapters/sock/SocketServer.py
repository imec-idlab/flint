import json
import asyncio
import logging

module_logger = logging.getLogger('SocketServer')

class ServerProtocol:
    def __init__(self, callback):
        self.logger = logging.getLogger('SocketServer')
        self.callback = callback

    def connection_made(self, transport):
        self.transport = transport
        self.client = None

    def datagram_received(self, data, addr):
        message = data.decode()
        self.logger.debug("received datagram")
        
        jmsg = json.loads(message)
        if("sink-ctrl" in jmsg):
            self.transport.sendto(data, addr)
            self.client = addr
        else:
            self.callback(jmsg)

    def sendto(self, data):
        message = json.dumps(data)

        if(self.client != None):
            self.transport.sendto(message.encode(), self.client)
        else:
            self.logger.error("Client not yet connected")

    def get_client(self):
        return self.client



class SocketServer():
    def __init__(self, address, port, callback):
        self.port = port
        self.address = address
        self.callback = callback

        self.logger = logging.getLogger('SocketServer')

    def publish(self, data):
        self.protocol.sendto(data)

    def run(self):
        loop = asyncio.get_event_loop()
        listen = loop.create_datagram_endpoint(lambda: ServerProtocol(self.callback), local_addr=(self.address, self.port))
        self.transport, self.protocol = loop.run_until_complete(listen)

        try:
            loop.run_forever()
        except KeyboardInterrupt:
            pass

        self.transport.close()
        loop.close()

