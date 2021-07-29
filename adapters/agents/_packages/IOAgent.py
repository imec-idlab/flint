from abc import ABC, abstractmethod
import configparser
import logging
import time

class IOAgent(ABC):
    # ------------ implemented and finished methods ------------ #
    
    # DO NOT touch these methods in derived classes
    
    def __init__(self, config, callback):
        self.connection_ip = config['agent']['io-agent-ip']
        self.port = int(config['agent']['io-agent-port'])
        self.user = config.get('agent', 'io-agent-user', fallback='')
        self.password = config.get('agent', 'io-agent-pw', fallback='')

        self.logger = logging.getLogger(config['agent']['name'])

        self.logger.info("initiating " + str(config['agent']['name']) + "...")
        self.initiate_agent(config, callback)

        self.connected = False


        attempts = 0
        while not self.connected and attempts < 3:
            maxTime = time.time() + 5 # wait for 5 seconds
            self.connecting = True
            attempts += 1
            self.connect()
            while self.connecting and time.time() < maxTime:
                pass
            self.connecting = False
            if time.time() >= maxTime:
                self.logger.error("server connection timeout, retrying...")
        if attempts == 3 and not self.connected:
            self.logger.critical("can't connect to server ")
            exit(1)

    def on_connect(self, *args):
        self.logger.info("agent connected");
        self.connecting = False
        self.connected = True

    def on_disconnect(self, *args):
        self.logger.info("agent disconnected")
        self.connected = False

    def from_downlink_to_central(self, message): # change to generic uplink function
        self.logger.debug('validating downlink packet')
        if not self.validate_downlink_packet(message):
            self.logger.debug("validating packet from downlink failed")
        else: 
            self.logger.debug('transforming downlink packet to central packet')
            msg = self.transform_downlink_to_central(message)
            self.logger.debug('handing packet over to other adapter')
            self.send_message_to_adapterbinding(msg)

    def from_central_to_downlink(self, message): # change to generic downlink function
        self.logger.debug('validating central packet')
        if not self.validate_central_packet(message):
            self.logger.debug("validating packet from central failed")
        else:
            self.logger.debug('transforming central packet to downlink packet')
            msg = self.transform_central_to_downlink(message)
            if self.connected:
                self.logger.debug('sending central packet downlink')
                print(msg)
                self.send_downlink(msg)
                self.logger.debug('packet sent')
            else:
                raise Exception("Not connected to a downlink server")



    # ------------ implement / override ------------ #

    # These methods can be overridden when needed 


    # implement instead of __init__ to set valuable information before a connection attempt is made
    def initiate_agent(self, config, callback):
        pass

    def validate_downlink_packet(self, message, *args):
        return True

    def validate_central_packet(self, message, *args):
        return True

    def transform_downlink_to_central(self, message, *args):
        return message

    def transform_central_to_downlink(self, message, *args):
        return message

    # These methods are mandatory to implement in derived adapters

    @abstractmethod
    def send_message(self, *args):
        pass

    @abstractmethod
    def on_message_received(self, *args):
        pass

    @abstractmethod
    def disconnect(self, *args):
        pass

    @abstractmethod
    def connect(self, *args):
        pass

