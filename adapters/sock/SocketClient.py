import asyncio
from random import randint

import socket
import json
import random
import time

import logging

module_logger = logging.getLogger('SocketClient')

class ClientProtocol:
    def __init__(self, loop, ip, port, callback):
        self.loop = loop
        self.ip = ip
        self.port = port
        self.transport = None
        self.logger = logging.getLogger('SocketClient')
        self.callback = callback

    def connection_made(self, transport):
        self.transport = transport
        message = { "sink-ctrl": { "socket": {"ip": self.ip, "port": self.port }}}
        message = json.dumps(message)
        self.transport.sendto(message.encode())

    def sendto(self, data):
        data = json.dumps(data)
        self.transport.sendto(data.encode(), (self.ip, self.port))

    def datagram_received(self, data, addr):
        self.logger.debug(f"received message from {addr}")
        data = json.loads(data.decode('utf8'))
        if("sink-ctrl" not in data):
            self.callback(data)


    def error_received(self, exc):
        self.logger.error(f"received {exc}")

    def connection_lost(self, exc):
        print("Socket closed, stop the event loop")
        self.loop = asyncio.get_event_loop()
        self.loop.stop()

class SocketClient:
    # The socket to write to
    client_socket = None

    # (ip, port)
    server_ip_and_port = None

    def __init__(self, ip, port):
        self.ip = ip
        self.port = port
        self.server_ip_and_port = (ip, port)
        self.connected = False

        self.logger = logging.getLogger('SocketClient')
        self.keep_running = False
        
        self.ping_thread = None
        self.run_thread = None

    def setup(self, callback):
        self.callback = callback

        self.loop = asyncio.get_event_loop()
        connect = self.loop.create_datagram_endpoint(lambda: ClientProtocol(self.loop, self.ip, self.port, self.callback), remote_addr=(self.ip, self.port))
        self.transport, self.protocol = self.loop.run_until_complete(connect)


    def close(self):
        self.transport.close()
        self.loop.close()

    def publish(self, data):
        self.protocol.sendto(data)