# HAS: Home Automation System
Class project for EEL5934 IoT Design; Building Automation System using ESP8266 and Raspberry Pi

## How to run the code
0. Install Docker using your favorite package manager or using the instructions on https://www.docker.com. The free Docker CE (Community Edition) is the correct version. Make sure you also install docker-compose.
1. Obtain this code (via zip file or from https://github.com/khaledhassan/HAS)
2. Configure the ```config/nodes.yml``` file to include the MAC addresses for each node you have.
3. `cd` to the `docker` directory and run the following command:
```./run build ; ./run up -d``` or ```./run build ; ./run up -d ; ./run logs -f``` to see the log messages. 
This starts the MQTT broker, the controller, and the web interface. The web interface is accessible at http://localhost:3000 or http://<Docker host's IP>:3000 if running on a different computer. You'll need the actual IP address to access remotely (like on a smartphone) and to configure the ESP node's firmware to talk to the MQTT broker. The MQTT broker listens on the default port of 1883.
4. Modify the ESP firmware to connect to the MQTT broker as described above. 

## Directory structure
Below is a proposed directory structure for this repository. Because the project should remain relatively simple overall, all components live together in the same repository.

```
├── config -- this directory will be mapped to the relevant containers (controller, web, etc) as a Docker volume
│   └──nodes.yml.example
├── controller
│   └──Dockerfile
│   └──other scripts used to run controller
├── docker
│   └──docker-compose.yml runs controller, mqtt broker, web interface
│   └──any Dockerfiles that define our external dependencies, (i.e. mosquitto) just in case they go away
│   └──perhaps amd64 compatible docker-compose.yml (and alternate Dockerfiles in controller/web directories)
├── esp
│   └──1 folder per node type containing Arduino sketches/code 
├── README.md (this file)
├── test
│   └──past experiments kept for reference (Arduino sketches, etc)
├── web
│   └──Dockerfile
│   └──other scripts/assets used to run web interface
```

## MQTT strategy
We are using the mosquitto MQTT broker, exposed as port 1883 on the Raspberry Pi's external network connection and the same port toward the other Docker containers.

ESP8266 nodes identify themselves by their WiFi MAC address, which is mapped using the ```config/nodes.yml``` file, used by both the web interface and the controller. 

The nodes publish their sensor data as JSON formatted messages on the topic ```sensor``` so that the controller can subscribe to ```sensor/#``` to catch all the sensors using a single subscription (using multi-level wildcard instead of single-level ```sensor/+``` so that the controller will see messages published to subtopics of ```sensor```).

The controller publishes commands on the topic ```actuator/<target MAC>``` such that each node is subscribed to it's own topic. 

Nodes shall publish a "join" message on the topic ```join_leave/<node MAC>``` according to the format defined below and shall register a last will and testament message on the same topic indicating "leave" so that the controller is aware of the nodes' status. 

The "join" message is published with the MQTT "retain" flag set to ```True``` so that the controller will see that each node is online, even if the node comes online before the controller. The controller will subscribe to ```join_leave/+``` to recieve these messages.

### JSON Message Definitions
All instances of the node MAC address will be in uppercase and with no colons. This applies in both the JSON body and the MQTT topic name.

All other variables and values in json are in lowercase.  

#### ```join_leave```
When connecting to the broker, each node publishes a message like the following:
```
{
  "mac": <node MAC>,
  "status": "join"
}
``` 
to the topic ```join_leave/<node MAC>``` with the "retain" flag set to ```True```.

Each node also registers a last will and testament message with the broker like the following:
```
{
  "mac": <node MAC>,
  "status": "leave"
}
```
to the same topic, with the "retain" flag set to ```False```. 

If a node connects and then disconnects before the controller starts up, the controller shouldn't see any message from that node. The last will message should "clear" the previous join message from the node's topic.

#### Light Node actuator (light on/off)
```
{
  "mac": <node MAC>,
  "type': "light",
  "action": "on/off"
}
```

#### Light Node sensor (motion detected)
```
{
  "mac": <node MAC>,
  "motion": "detected" (though the value doesn't really matter)
}
```

#### Light Node status query (published on actuator subtopic)
This message is used by the controller to determine light state on controller startup (e.g. if the controller restarts while the lights are operating)
```
{
  "mac": <node MAC>,
  "type': "light",
  "action": "status"
}
```

#### Light Node status response (published on sensor subtopic)
```
{
  "mac": <node MAC>,
  "type': "light",
  "state": true/false
}
```

#### AC Node actuator (fan on/off)
```
{
  "mac": <node MAC>,
  "type': "ac",
  "action": "on/off"
}
```

#### AC Node sensor (temperature/humidity)
```
{
  "mac": <node MAC>,
  "type": "ac", 
  "t": "75", 
  "h": "60"
}
```

#### Door Lock Node actuator
```
{
  "mac": <node MAC>,
  "type": "door",
  "action": "unlock" // only one action, that is unlock the door
}
```

#### Door Lock Node sensor (status: locked/unlocked)
```
{
  "mac": <node MAC>,
  "type": "door",
  "status": "locked/unlocked"
}
```

###JSON format between Websocket and frontend 
To be added. 
