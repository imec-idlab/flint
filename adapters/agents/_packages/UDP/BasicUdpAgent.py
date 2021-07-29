# pip3 install requests Flask
from ..IOAgent import IOAgent
from abc import ABC

import socket

import platform
import subprocess 

import json

import logging

module_logger = logging.getLogger('BasicUdpAgent')

class BasicUdpAgent():
    def __init__(self, config):
        self.logger = logging.getLogger('BasicUdpAgent')

        self.UDP_IP_ADDRESS_SERVER = config['agent']['io-agent-ip']
        self.UDP_PORT_SERVER = int(config['agent']['io-agent-port'])

    def send_message(self, msg):
        try:
            client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
            self.logger.debug("Sending message to UDP server on {} port {} ...".format(self.UDP_IP_ADDRESS_SERVER, self.UDP_PORT_SERVER))
            client.sendto(msg.encode('utf-8'), (self.UDP_IP_ADDRESS_SERVER, self.UDP_PORT_SERVER))
        except Exception as e:
            self.logger.error("failed to parse or send message: " + str(e))