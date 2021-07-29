import signal
import time
import platform
import argparse
import threading
import configparser

import paho.mqtt.client as mqtt
from sock.SocketClient import *
from helpers.Validator import *

from abc import ABC, abstractmethod
import json
import logging


class Agent(ABC):
    def __init__(self, name):
        self.argparser = argparse.ArgumentParser()
        self.argparser.add_argument("-f", "--config-file", help="specify the config file. For an example see config.ini",
                               default="config.ini")
        self.argparser.add_argument("-l", "--logging-mode", help="specify log level (DEBUG, INFO, WARNING, ERROR, CRITICAL)",
                               default="WARNING")

        self.logger = None
        self.arg_config = None
        self.config = None

        self.name = name
        self.client = None

        self.adapter_id = None
        self.message_scheme = None

        f = open('helpers/message_scheme.json', 'r')
        self.message_scheme = json.loads(f.read())
        f.close()

    def get_argparser(self):
        return self.argparser

    def parse_arguments(self):
        self.arg_config = self.argparser.parse_args()

        numeric_level = getattr(logging, self.arg_config.logging_mode.upper(), None)
        if not isinstance(numeric_level, int):
            print("wrong logging attribute, using WARNING")
            numeric_level = logging.WARNING

        logging.basicConfig(level=numeric_level)
        self.logger = logging.getLogger(self.name)
        self.set_config(self.arg_config.config_file)

        return self.arg_config

    def get_logger(self):
        return self.logger

    def set_config(self, config_file):
        self.config = configparser.ConfigParser()
        self.config.read(config_file)

        self.logger.info("using configfile " + config_file)

        try:
            self.validator = Validator(config_file)
            self.adapter_id = str(self.config["sink"]["id"])
        except ValidatorException as e:
            self.logger.error(e)
            exit()

    def get_config(self):
        return self.config

    def get_socket_client(self):
        return self.client

    def connect_sink_socket(self):
        SOCKET_SERVER_IP = self.config['socket']['socket-ip']
        SOCKET_SERVER_PORT = int(self.config['socket']['socket-port'])
        self.client = SocketClient(SOCKET_SERVER_IP, SOCKET_SERVER_PORT)
        self.client.setup(self.socket_receive_callback)

        return self.client

    def validate_message_structure(self, msg):
        validate(instance=msg, schema=self.message_scheme)

    @abstractmethod
    def socket_receive_callback(self, msg):
        pass

    def run(self):
        keep_running = True
        while keep_running:
            try:
                self.client.loop.run_forever()
                if platform.system() == "Windows":
                    time.sleep(1)
                else:
                    signal.pause()
            except KeyboardInterrupt:
                self.logger.warning("received KeyboardInterrupt... stopping processing")
                self.client.disconnect()
                keep_running = False