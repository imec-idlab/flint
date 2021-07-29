# FLINT: Flows for the Internet of Things
## ABOUT FLINT

FLINT is a flexible, modular and scalable network architecture for interconnecting IoT devices, networks, middleware and platforms. A FLINT configuration can be built from fine-grained components on a per device basis. These components are data processingelements calledadapters. An adapter consists of two sub-elements; an agent and a sink. The sink connects the adapter to the platform by means of the Message Broker hub node.The Message Broker provides a message bus for efficient data delivery using the publish/subscribe paradigm. The sink and the node communicate over a socket interface so that the node can be built from any programming language. This allows users to recycle their existing programs and implementations. A FLINT configuration can be built by chaining adapters. The user chooses a collection of adapters for a given device and connects them into a chain that represents the path the data will follow.

See the [examples](/adapters/examples) and [our evaluation paper](http://hdl.handle.net/1854/LU-8613162) for more information on the implemenation and configuration.

### ACKNOWLEDGEMENT

FLINT has been developed in the scope of the European Union’s Horizon 2020 PortForward project, where, amongst others, LwM2M will be integrated with this platform in order to deliver open standards-based sensor-Cloud connectivity.

## LICENSE
FLINT is released under the LGPLv3.0. The GNU Lesser General Public License, Version 3 (the "License") is the open source license: You may not use these files except in compliance with the License. You may obtain a copy of the License at <https://www.gnu.org/licenses/lgpl-3.0.html>

See the License for the specific language governing permissions and limitations under the License.

© Copyright 2020-2021, Bart Moons <bamoons.moons@ugent.be>, imec and Ghent University
