# pip3 install requests Flask
from ..IOAgent import IOAgent
from abc import ABC

from http.server import HTTPServer, BaseHTTPRequestHandler
from http.client import HTTPSConnection, HTTPConnection
from base64 import b64encode

import sseclient
import requests
import threading

import platform
import subprocess 

import pprint
import cgi
import json

import logging

IOAgent = IOAgent

module_logger = logging.getLogger('BasicHttpAgent')

#https://stackoverflow.com/questions/2953462/pinging-servers-in-python
def ping(host):
    """
    Returns True if host (str) responds to a ping request.
    Remember that a host may not respond to a ping (ICMP) request even if the host name is valid.
    """
    # Option for the number of packets as a function of
    param = '-n' if platform.system().lower()=='windows' else '-c'
    if(host[0] == '['): # ipv6 address
        host = host[1:40]
        command = ['ping', param, '1', '-6', host]
    else:
        # Building the command. Ex: "ping -c 1 google.com"
        command = ['ping', param, '1', host]
    return subprocess.call(command) == 0

""" 
app = Flask(__name__)

@app.route("/<url>", methods = ['POST'])
def http_endpoint(url):
    return {
        "message": "request OK"
    }, 200 """


#todo: make child classes for specific adapters for data validation/transformation depending on use-case

class BasicHttpAgent(IOAgent, ABC):
    def __init__(self, *args):
        super().__init__(*args)

        self.logger = logging.getLogger('BasicHttpAgent')

        self.connection = None
        self.headers = {}

    class SimpleHTTPRequestHandler(BaseHTTPRequestHandler):
        def __init__(self, adapter, *args):
            self.adapter = adapter
            super().__init__(*args)

        def set_headers(self, statuscode):
            self.send_response(statuscode)
            self.send_header('Content-type', 'application/json')
            self.end_headers()

        #https://gist.github.com/nitaku/10d0662536f37a087e1b
        def do_POST(self):
            ctype, pdict = cgi.parse_header(self.headers.get('content-type'))
            
            # refuse to receive non-json content
            if ctype != 'application/json':
                self.set_headers(400)
                return
                
            # read the message and convert it into a python dictionary
            length = int(self.headers.get('content-length'))
            message = json.loads(self.rfile.read(length))
            
            # send the message back
            self.set_headers(200)
            self.wfile.write(str.encode(json.dumps({'received': 'ok'})))
            self.adapter.receive_from_downlink(message)

    def initiate_agent(self, config, callback):
        self.connection_host = f"{self.connection_ip}:{self.port}"
        self.connection_url = f"http://{self.connection_host}" # + config url
        self.host_url = "/" # + config

        self.message_received_callback = callback

        # https://stackoverflow.com/questions/18444395/basehttprequesthandler-with-custom-instance
        # def handler(*args):
        #    return BasicHttpAdapter.SimpleHTTPRequestHandler(self, *args)
        # self.httpd = HTTPServer(('localhost', 5000), handler)
        # threading.Thread(target=self.httpd.serve_forever).start()
    
    def send_downlink(self, message, *args):
        requests.post(self.connection_url, message)

    
    def receive_from_downlink(self, message):
        #if url == self.host_url:
        # parse incomming message
        self.from_downlink_to_central(message)
            

    def disconnect(self, *args):
        self.on_disconnect()

    def connect(self, *args):
        # if ping(self.connection_ip):
        self.on_connect()

    def set_headers(self, headers):
        self.headers = headers

    def basic_auth(self, uri):
        self.connection = HTTPSConnection(f"{self.connection_ip}:{self.port}")
        # authenticate with client_id and client_secret
        auth_string = b64encode(bytes(self.user + ':' + self.password, "utf-8")).decode("ascii")
        headers = { 
            'Content-type': "application/x-www-form-urlencoded", 
            'Authorization': 'Basic %s' %  auth_string 
        }
        body = f"grant_type=client_credentials"
        self.connection.request('POST', f'/{uri}', headers=headers, body=bytes(body, encoding="utf-8"))
        res = self.connection.getresponse()
        data = res.read()

        self.logger.debug("successfully authenticated")

        return data

    def rpt_auth(self, uri, access_token):
        self.connection = HTTPSConnection(f"{self.connection_ip}:{self.port}")

        # authenticate with access token
        headers = { 
            'Content-type': "application/x-www-form-urlencoded", 
            'Authorization': 'Bearer %s' %  access_token 
        }
        body = f"grant_type=urn:ietf:params:oauth:grant-type:uma-ticket&audience=policy-enforcer"
        self.connection.request('POST', f'/{uri}', headers=headers, body=bytes(body, encoding="utf-8"))
        res = self.connection.getresponse()
        data = res.read()

        self.logger.debug("successfully got RTP token")

        return data

    def refresh_rpt(self, uri, refresh_token):
        self.connection = HTTPSConnection(f"{self.connection_ip}:{self.port}")

        # authenticate with access token
        headers = { 
            'Content-type': "application/x-www-form-urlencoded"
        }
        body = f"grant_type=refresh_token&refresh_token={refresh_token}&client_id={self.user}&client_secret={self.password}"
        
        self.connection.request('POST', f'/{uri}', headers=headers, body=bytes(body, encoding="utf-8"))
        res = self.connection.getresponse()
        data = res.read()

        self.logger.debug("successfully refreshed token")

        return data

    def get_stream(self, uri):
        try:
            headers = { 
                'Accept': "text/event-stream"
            }
            response = requests.get(uri, stream=True, headers=headers)
            client = sseclient.SSEClient(response)
            for event in client.events():
                # pprint.pprint(json.loads(event.data))
                self.message_received_callback(event.data)
        except Exception as e:
            self.logger.error("failed to parse message: " + str(e))

    def on_message_received(self, client, config, msg):
        self.logger.debug("received message")
        data = msg.payload
        try:
            if isinstance(data, str):
                payload = json.loads(json.dumps(data))
            else:
                payload = json.loads(data)

                self.message_received_callback(payload)

        except Exception as e:
            self.logger.error("failed to parse message: " + str(e))

    def send_message(self, uri, msg, method):
        self.connection = HTTPConnection(host=self.connection_ip, port=self.port, timeout=128)
        try:
            self.connection.request(method, f'/{uri}', body=msg, headers=self.headers)
            res = self.connection.getresponse()
            data = res.read()

            self.logger.debug(method + " " + uri + " returned " + str(res.status))

            self.connection.close()

            return res.status, data
        except Exception as e:
            self.logger.error("failed to parse or send message: " + str(e))



    def send_secure_message(self, uri, msg, token):
        self.connection = HTTPSConnection(f"{self.connection_ip}:{self.port}")
        try:
            headers = { 
                'Authorization': 'Bearer %s' % token
            }
            
            self.logger.debug(msg)

            self.connection.request('POST', f'/{uri}', headers=headers, body=msg)
            res = self.connection.getresponse()
            data = res.read()

            self.logger.debug(data)

            self.connection.close()

            return res.status
        except Exception as e:
            self.logger.error("failed to parse or send message: " + str(e))
