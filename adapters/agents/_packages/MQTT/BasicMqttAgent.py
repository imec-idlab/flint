import paho.mqtt.client as mqtt
from abc import ABC, abstractmethod
import configparser
import json
import collections
import logging

from ..IOAgent import IOAgent
IOAgent = IOAgent

module_logger = logging.getLogger('BasicMqttAgent')

#todo: make child classes of specific adapters for data validation/transformation depending on use-case
class BasicMqttAgent(IOAgent, ABC):
    def __init__(self, *args):
        super().__init__(*args)

        self.logger = logging.getLogger('BasicMqttAgent')

    def initiate_agent(self, config, callback):
        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_disconnect = self.on_disconnect
        self.client.on_message = self.on_message_received

        self.message_received_callback = callback
        
        self.publish_topic = f"{config['agent']['pub-topic']}"
        self.subscribe_topic = f"{config['agent']['sub-topic']}"

        self.sub_qos = 0
        self.pub_qos = 0

        if("sub-qos" in config['agent']):
            self.sub_qos = int(config['agent']['sub-qos'])
        if("pub-qos" in config['agent']):
            self.pub_qos = int(config['agent']['pub-qos'])

    def on_connect(self, client, userdata, flags, rc):
        self.logger.info("connected");
        self.client.subscribe(self.subscribe_topic, qos=self.sub_qos)
        self.connecting = False
        self.connected = True
        self.logger.info("subscribed to topic %s", self.subscribe_topic)

    def on_disconnect(self, client, userdata, rc):
        self.logger.info("disconnected")
        self.connected = False

    def connect(self):
        if self.connected: 
            return
        try:
            self.logger.info(f"connecting with {self.connection_ip}:{self.port}")
            if self.user != "" and self.password != "":
                self.client.username_pw_set(self.user, self.password)
            self.client.connect(self.connection_ip, self.port, keepalive=60)
            self.client.loop_start()
            self.client.subscribe(self.subscribe_topic, qos=self.sub_qos)
        except:
            self.logger.error("failed to connect to MQTT broker")
            raise
    
    def disconnect(self, *args):
        self.client.disconnect()

    def on_message_received(self, client, config, msg):
        self.logger.debug("received message")
        flag = False
        data = msg.payload
        try:
            if isinstance(data, str):
                payload = json.loads(json.dumps(data))
            else:
                payload = json.loads(data)

                self.message_received_callback(payload)

        except Exception as e:
            print("failed to parse message: " + str(e))

    def send_message(self, msg, pub_topic=None):
        
        topic = pub_topic if pub_topic is not None else self.publish_topic
        try:
            data = json.dumps(msg)
            self.client.publish(str(topic), str(data), self.pub_qos)
            self.logger.debug(f"sent packet on {topic}")
        except Exception as e:
            self.logger.error("failed to parse or send message: " + str(e))