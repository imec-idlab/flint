import signal
import time
import platform
import paho.mqtt.client as mqtt
from sock.SocketClient import *
from helpers.Validator import *
from agents.Agent import Agent

import os
import json
from enum import Enum

class Direction(Enum):
   up = 1
   down = 2

class Mapper(Agent):
    def __init__(self):

        super().__init__("Mapper")

        # first add custom arguments
        self.argparser = self.get_argparser()
        self.argparser.add_argument("-td", "--thing-descriptions", help="specify the folder that contains the thing descriptions",
                               default="mapper/thing_descriptions")
        self.argparser.add_argument("-ad", "--adapter-definitions", help="specify the file that contains the adapter definitions",
                               default="mapper/adapter_definitions.json")
        
        # otherwise just parse
        self.arg_config = self.parse_arguments()
        self.logger = self.get_logger()

        self.mapper_id = str(self.config['sink']['id'])

        # connect to the sink over the socket
        self.client = self.connect_sink_socket()

        # read the adapter and thing descriptions
        self.adapters = self.parse_adapters()
        self.things = self.parse_thing_descriptions()

    def parse_adapters(self):
        f = open(self.arg_config.adapter_definitions, "r")
        try:
            adapters = []
            adapter_definitions = json.loads(f.read())

            # todo validate using adapter scheme
            for ad in adapter_definitions:
                adapters.append(ad)

            f.close()
            self.logger.info("adapters available from %s", self.arg_config.adapter_definitions)

            return adapters
        except Exception as e:
            self.logger.error("failed to parse file: " + str(e))


    def parse_thing_descriptions(self):
        folder = self.arg_config.thing_descriptions
        files = [f for f in os.listdir(folder) if (os.path.isfile(os.path.join(folder, f)) and f.endswith('.json'))]
    
        things = []
        self.logger.info("using %s from folder %s", files, self.arg_config.thing_descriptions)

        try:
            for path in files:
                thing = {}
                td = open( f"{self.arg_config.thing_descriptions}/{path}", "r")
                json_td = json.loads(td.read())
                # todo validate using td scheme
                if( ("id" not in json_td) or ("title" not in json_td) or ("adapterScheme" not in json_td) ) :
                    self.logger.error("not a valid thing description")
                else:
                    thing['id'] = json_td['id']
                    thing['title'] = json_td['title']
                    thing['adapterScheme'] = json_td['adapterScheme']
                    if("ioAdapterDefinitions" in json_td):
                        thing['ioDefinitions'] = self.parse_io_definitions(json_td['ioAdapterDefinitions'])
                    if("ipv6" in json_td):
                        thing['ipv6'] = json_td['ipv6']

                if(self.thing_is_unique(things, thing)):
                    things.append(thing)
                else:
                    self.logger.error("thing with id %s already exists", thing['id'])
                td.close()

            return things

        except Exception as e:
            self.logger.error("failed to construct thing description from file: " + str(e))

    def parse_io_definitions(self, adapter_definitions):
        network_input = []

        for definition in adapter_definitions:
            definition["active"] = 0
            if("mac" not in definition["deviceDefinitions"]):
                definition["deviceDefinitions"]["mac"] = "0000000000000000"
            network_input.append(definition)

        return network_input

    def construct_route(self, adapter_scheme):
        scheme = []
        for input_adapter in adapter_scheme[0]: # FIRST HOP: input to mapper
            route = {}
            route[input_adapter] = self.mapper_id
            scheme.append(route)
        for next_adapter in adapter_scheme[1]: # SECOND HOP: mapper to second hop
            route = {}
            route[self.mapper_id] = next_adapter
            scheme.append(route)
        for i, s in enumerate(adapter_scheme): # loop over all hops
            for j, adapter in enumerate(s): # all adapters in current hop
                if( (i < (len(adapter_scheme) - 1)) and (i >= 1)): # ALL OTHER HOPS
                    for next_adapter in adapter_scheme[i + 1]: # all adapters in next hop
                        route = {}
                        route[adapter] = next_adapter
                        scheme.append(route)
        return scheme


    def thing_is_unique(self, things, thing):
        for t in things:
            if(t['id'] == thing['id']):
                return 0
        return 1

    def get_thing_by_dev_eui_and_adapter_id(self, dev_eui, adapter_id):
        if dev_eui == "0000000000000000":
            return None
        for thing in self.things:
            io_adapter_definitions = thing["ioDefinitions"]
            for definition in io_adapter_definitions:
                if( (dev_eui == definition["deviceDefinitions"]["mac"]) and (adapter_id == definition["uuid"]) ):
                    return thing

    def get_thing_by_ipv6(self, ipv6_address):
        for thing in self.things:
            for ipv6 in thing["ipv6"]:
                if ipv6 == ipv6_address:
                    return thing

    def set_active_input_adapter(self, dev_eui, adapter_id):
        for thing in self.things:
            io_adapter_definitions = thing["ioDefinitions"]
            for definition in io_adapter_definitions:
                if( (dev_eui == definition["deviceDefinitions"]["mac"])):
                    definition["active"] = 0 # set other instances to inactive for *this* thing
                    if(adapter_id == definition["uuid"]):
                        self.logger.info("set %s as active adapter for device %s", adapter_id, dev_eui)
                        definition["active"] = 1 # set the current input adapter to active

    def get_active_input_adapter(self, thing):
        io_adapter_definitions = thing["ioDefinitions"]
        for definition in io_adapter_definitions:
            if(definition["active"] == 1):
                return definition

    def get_chains_for_adapter(self, thing, adapter_id):
        chains = []
        if(thing and "adapterScheme" in thing):
            scheme = thing["adapterScheme"]
            for chain in scheme: # for every chain
                for adapter in chain[0]: # get the adapters in the first shackle
                    if(adapter == adapter_id):
                        chains.append(chain) # all chains that have adapter_id in the first shackle
        else:
            self.logger.error("no thing provided to get chains")
        return chains

    def adapter_in_chain(self, chain, adapter):
        for shackle in chain:
            for adapter_uuid in shackle:
                if adapter and (adapter_uuid == adapter['uuid']):
                    return True
        return False

    def socket_receive_callback(self, json_msg):
        # mapper request from adapter        
        adapter_exists = False
        dev_eui = json_msg["device-ctrl"]["dev-eui"]
        adapter_id = json_msg["adapter-ctrl"]["from"]
        thing = self.get_thing_by_dev_eui_and_adapter_id(dev_eui, adapter_id)

        for adapter in self.adapters:
            if adapter["uuid"] == adapter_id:
                self.logger.info(f"request from adapter {adapter_id} for device with eui {dev_eui}")
                adapter_exists = True
                break

        if not adapter_exists:
            return

        direction = None

        if thing == None:
            if("output-ctrl" in json_msg): # first case: request in downward direction from output adapter
                if "ipv6" in json_msg["output-ctrl"]:
                    ipv6_address = json_msg["output-ctrl"]["ipv6"]
                    thing = self.get_thing_by_ipv6(ipv6_address)
                    direction = Direction.down
                if(thing == None):
                    self.logger.error("no device available for the given ipv6 address")
                    return
        else:
            if "output-ctrl" in json_msg:
                if "direction" in json_msg["output-ctrl"]:
                    if json_msg["output-ctrl"]["direction"] == "DOWN": # second case: request from processing adapter in downward direction
                        direction = Direction.down
                    else: # third case: request from input adapter
                        direction = Direction.up
            else:
                self.logger.debug("request from adapter without specified direction")
            
            ipv6_address = "0:0:0:0:0:0:0:0"

        # get chains that start with the requesting adapter
        adapter_scheme = []
        chains = self.get_chains_for_adapter(thing, adapter_id)

        for chain in chains:
            if direction == Direction.up: # request from input adapter
                adapter_scheme = chain
                self.set_active_input_adapter(dev_eui, adapter_id)
                # todo
                # check downlink type and flush downlink queue accordingly
            
            else: # request from output adapter
                active_adapter = self.get_active_input_adapter(thing) # get the last active adapter
                
                if(self.adapter_in_chain(chain, active_adapter)): # active adapter part of the chain?
                    # todo check downlink queue
                    json_msg["device-ctrl"]["dev-eui"] = active_adapter["deviceDefinitions"]["mac"]
                    json_msg["input-ctrl"] = {
                        "input-custom-ctrl": active_adapter["deviceDefinitions"]
                    }
                    
                    # only keep active input adapter in the adapter scheme
                    for i, sh in enumerate(chain):
                        shackle = []
                        if(i < len(chain) - 1): # keep all but the last shackle
                            shackle = sh
                        else:
                            shackle.append(active_adapter["uuid"]) # replace the entire last shackle with the active adapter
                        adapter_scheme.append(shackle)
                else:
                    # active adapter not in chain
                    adapter_scheme = chain

            # add output ctrl for this thing
            json_msg["output-ctrl"]["ipv6"] = ipv6_address;
            json_msg["output-ctrl"]["thing-id"] = thing["id"];

            # add input/output definitions for this thing
            if("ioDefinitions" in thing):
                adapters = []
                for output_adapter in thing["ioDefinitions"]:
                    adapter = {}
                    adapter[output_adapter["uuid"]] = output_adapter
                    adapters.append(adapter)

                json_msg["output-ctrl"]["output-custom-ctrl"] = adapters

            # construct the route
            json_msg["adapter-ctrl"]["adapter-scheme"] = self.construct_route(adapter_scheme)

            # and forward to the next adapter(s)
            self.client.publish(json_msg)


if __name__ == "__main__":
    Mapper().run()
