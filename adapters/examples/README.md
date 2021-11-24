# Examples
The examples incrementally build from local deployments towards Kubernetes deployments. They give an idea about the individual components and how they interact with each other. Information about creating your own adapter can be found below.

# Adapters
A sink and an agent together form an adapter. The sink is a reusable component that connects to the FLINT network. The Agent is a case specific program which connects to the sink over a socket connection. Different kinds of adapters exist:
* I/O Adapter: an adapter at the end of a chain. This adapter either pulls information (IN) or pushes it to another source (OUT).
* Processing Adapter: processing adapters sit between I/O adapters. They can be used to add extra services to a specific flow. 
* Direct Adapter: a direct adapter forwards data to the destination adapter directly (i.e. without the use of the mapper adapter).

## Sink
The sink implementation is written in C and can be built locally. It uses a configuration file to load the sink specific configuration parameters.

## Agent
Agents can be written in any programming language. They connect with the sink over a local socket connection. The Agent must adhere to FLINT's data model, an example is given below.

4 fields must be present in order to comply with the data structure: `device-ctrl`, which is used to distribute information about the device that first send the data. The `input-ctrl` field contains information about the adapter that first received the information. Every I/O adapter must add its `uuid` to the `from` field in the `adapter-ctrl` section. The input adapter forwards the messages (when the mapper adapter is used) to the Mapper, which will complement the `adapter-ctrl` field with the generated adapter scheme.

### Custom Fields
Some sections, such as `device-ctrl` and `output-ctrl` have custom fields. These are called, respectively, `device-custom-ctrl` and `output-custom-ctrl`. Use case specific data can be added to the `device-custom-ctrl` field. Information that should be addressed specifically to the output adapter, can be added to the `output-custom-ctrl` field. In the below case, the Mapper adds the available network connections of the device to provide context information to the output adapter.

### REST
Some agents require a more sophisticated way of communicating than just push/pull. Therefore, the `method` field is present in the `output-ctrl`. Every agent can take action based on the input of this field. In order to request a specific action from the output adapter, the input adapter can set this field to one of the following:
* GET
* PUT
* POST
* DELETE
* PATCH

### Example
``` JSON
{
    "device-ctrl":
    {
        "data": "BY4RQqBIuk3cCfmPzbCAQSwA",
        "dev-eui": "0000000000000001",
        "device-custom-ctrl": { }
    },
    "input-ctrl":
    {
        "input-type": "AgentName",
        "input-ip": "192.168.0.100",
        "input-port": 1883,
        "input-protocol": "mqtt"
    },
    "output-ctrl":
    {
        "method": "POST",
        "ipv6": "0:0:0:0:0:0:0:0",
        "thing-id": "urn:uuid:91f03ebe-5293-436b-a425-36d825e02e64",
        "output-custom-ctrl":
        [
            {
                "urn:uuid:cc0c1e3b-ce09-4c44-b9da-57ee9871497a":
                {
                    "uuid": "urn:uuid:cc0c1e3b-ce09-4c44-b9da-57ee9871497a",
                    "deviceDefinitions":
                    {
                        "mac": "0000000000000001",
                        "interfaceType": "continuous",
                        "networkDefinitions":
                        {
                            "fport": 10
                        }
                    },
                    "active": 1
                }
            },
            {
                "urn:uuid:489edd56-4b1c-4332-a5ba-cad14994668d":
                {
                    "uuid": "urn:uuid:489edd56-4b1c-4332-a5ba-cad14994668d",
                    "deviceDefinitions":
                    {
                        "interfaceType": "uplink_triggered",
                        "mac": "92BC10"
                    },
                    "active": 0
                }
            }
        ]
    },
    "adapter-ctrl":
    {
        "from": "urn:uuid:cc0c1e3b-ce09-4c44-b9da-57ee9871497a",
        "adapter-scheme":
        [
            {
                "urn:uuid:cc0c1e3b-ce09-4c44-b9da-57ee9871497a": "urn:uuid:e4370d7c-eae3-416d-85f0-751d396cacf5"
            },
            {
                "urn:uuid:489edd56-4b1c-4332-a5ba-cad14994668d": "urn:uuid:e4370d7c-eae3-416d-85f0-751d396cacf5"
            }
        ]
    }
}

```