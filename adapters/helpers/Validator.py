import json
import logging
from jsonschema import validate

from configparser import ConfigParser


module_logger = logging.getLogger('Sink.Validator')

class ValidatorException(Exception):
    pass

class Validator(ConfigParser):
    def __init__(self, config):
        super(Validator, self).__init__()
        self.logger = logging.getLogger('Sink.Validator')

        self.logger.info("using configfile " + config)

        self.read(config)
        self.validate_config()

    def validate_config(self):
        required_values = {
            "sink": {
                "broker-ip": 0,
                "broker-port": 0,
                "adapter-type": "type"
            },
            "socket" : {
                "socket-ip": 0,
                "socket-port": 0
            }
        }

        for section, keys in required_values.items():
            if section not in self:
                raise ValidatorException('missing section %s in the config file' % section)

            for key, values in keys.items():
                if key not in self[section] or self[section][key] == '':
                    raise ValidatorException((
                        'missing value for %s under section %s in ' +
                        'the config file') % (key, section))

        self.logger.info("config file ok")

    def get_json(self, packet):
        packet = str(packet).replace('"','\"').replace("\'", "\"") # escape quotes first, than make string
        json_message = json.loads(packet)

        return json_message


    def get_config(self):
        return self


    def set_adapter_info(self, packet):
        # update packet with broker adapter agent information
        message = {}

        message["central-broker-ctrl"] = {
            "central-broker-ip": self['broker-adapter-agent']['adapter-agent-ip'],
            "central-broker-port" : int(self['broker-adapter-agent']['adapter-agent-port']),
            "central-broker-protocol": "mqtt",
            "central-broker-topic-ctrl": {
                "pub-topic": self['broker-adapter-agent']['pub-topic'],
                "pub-topic-direction": self['broker-adapter-agent']['pub-topic-direction'],
                "sub-topic": self['broker-adapter-agent']['sub-topic'],
                "sub-topic-direction": self['broker-adapter-agent']['sub-topic-direction'],
            },
            "adapter-ctrl": {
                "adapter-id" : int(self['broker-adapter-agent']['id'])
            }
        }
        message = json.dumps(message)

        packet.update(json.loads(message))

        return packet