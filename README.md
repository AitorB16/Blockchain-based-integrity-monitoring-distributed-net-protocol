# Distributed integrity monitoring system based on Blockchain technology
The aim of the project is to create a multi threaded **network protocol** that monitorizes the integrity of other nodes in a **distributed** way; a **Byzantine fault** tolerant **P2P** network.
A digest describing a filesystem is received from [*File integrity monitoring on linux systems*](http://github.com/aritzherrero4) underlayer, OS level, project and is spreaded to other network nodes. Thus, the nodes will be able to update their local **Blockchains** associated with the sender.
At the same time, each node will audit other nodes randomly, compraing the received hash with the one stored in local dependencies; each node has the whole structure of the network replicated.
[Refer to full doucmentation](./documentation.pdf)

## Directory structure
- **deploy_utils (deploying scripts and tools):** XML generator python script, C++ RSA
keys generator application.
- **include (dependent packages):** Rapid-XML package.
- **logs (log history):** Log files generated during the execution.
- **RSA_keys (RSA keys):** Public keys corresponding to all network nodes and keypair
of self node.
- **src (main development):** application headers, source files, makefile and network
deploying shell script.
- **XML_config (configurations):** XML configuration file.

## Compilation instructions
To compile use ```make```. To perform a clean build use ```make clean && make```.
> _it lives in [src/Makefile](src/Makefile)_

### Install dependencies
Install the required libraries:
```sh
sudo apt-get update
sudo apt-get install build-essential libcrypto++-dev libcrypto++-doc libcrypto++-utils libsystemd-dev
```
> Note: Linux kernel has to run systemd in order to the linker to work.

## Previous configurations
The minimum amount of nodes to deploy the system is **3**: Self node + two network nodes.

Before executing the program, key distribution has to be performed. Each node has to know public keys of other nodes as well as have a public and private key-pair. 
> Note: Key format: RSA DER (binary).

### Key distribution
It is recommended to use a PKI infrastructure to perfrom key distribution. However a C++ key generator program has been created.

#### Key generator program
The application generates a RSA 2048 bit key-pair for a given argv (positive integer ID) node.
To compile and run the program:
```sh
g++ -o deploy_utils/generate_keys deploy_utils/generate_keys.cpp -lcryptopp
./generate_keys/deploy_utils <ID>
```

### Configuration file
Each node has to be configured by a system administrator using a **XML** configuration file, following the structure:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<config>
    <!-- execution modes - 0: DEFAULT - basic output / 1: DEBUG - full output -->
    <execution_mode />
    <!-- sha256sum of the desired passwd -->
    <passwd_sha256 />
    <node_self>
        <id />
        <ip />
        <port />
        <pub />
        <prv />
    </node_self>
    <network>
        <node>
            <id />
            <ip />
            <port />
            <pub />
        </node>
        <node />
        <node />
        ...
    </network>
</config>
```
> _it lives in [XML_config/config.xml](XML_config/config.xml)_

#### Configuration generator Python scrpit
To automatically generate multiple XML files [deploy\_utils/generate\_configs.py](deploy\_utils/generate\_configs.py):
```sh
./generate_keys/generate_configs.py <totalNodeNumber> <ID>
```
## Optional configurations
There are multiple variables associated to the protocol behaviour set by default, in [globals.hpp](src/globals.hpp), specially the ones associated with delays. Depending on the network size, tunning them could be a necessary:
- TIME_SPACE_BEFORE_AUDIT 60 sec.
- DEPLOY_TIME 30 sec.
- RESPONSE_DELAY_MAX sec.
- HASH_UPDATE_TIMESPACE_MAX 300 sec.
- HASH_UPDATE_TIMESPACE_MIN 30 sec.
- AUDITOR_INTERVAL 3 sec.

> Note: The application has to be recompiled after performing any change.

## Execution instructions
Depending on the specified argv parameters the application has two execution variants.

### Normal deploy 
The normal scenario will handle an instance of the application per remote node. In this case, the ID argv parameter is **optional** because it is previously setted in the configuration file:
```sh 
./src/run <ID>
```
### Multi local test deploy
To test the network deploy in a local computer a shell script [deploy\_utils/deploy.sh](deploy\_utils/deploy.sh) has been created. The script will create the neccessary files, keys and XMLs, calling to the previous Python script and C++ application. Moreover, it will open a shell tab per execution instance. The script **must** be executed in the following order.

#### Generator mode
To create the RSA keys and configuration files (if no manual configuration is previously done):
```sh 
./src/deploy.sh <totalNodeNumber> g
```

#### Execution mode
To execute the multiple instances of the protocol in independent interfaces:
```sh 
./src/deploy.sh <totalNodeNumber>
```

## Usage instructions
Once the network is deployed and consistency achieved in all the nodes, a menu with four options will be displayed:
- 0 - Close the program.
- 1 - Print network overview.
- 2 - Print specific info of a node.
- 3 - Update self hash, work-frame.

### Network overview info
Displays network trustiness, whether each node is trusted or not as well as the latest valid hash and blockchain digest of them.
### Specific node info
A specific node ID is requested to display a complete breakdown of all its information: Trustiness in the node, complete good, bad and blockchain records.

### Open a working time-frame
The plain text corresponding to the entered SHA256 password (XML) is requested as well as a bounded (globals) working time to stop the auditor and notify other nodes.

## Linked OS level implementation
The application is located in a particular context, where an underlayer OS level application elaborates a hash from a tracked filesystem using **Merkle Tree**. The network protocol should be running alongside the OS daemon.

To more information refer to [File integrity monitoring on linux systems](http://github.com/aritzherrero4) repository.

## Author

* **Aitor Belenguer** 

## License

MIT
