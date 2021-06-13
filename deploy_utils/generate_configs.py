#!/bin/python
import sys

import xml.etree.cElementTree as ET

ident = int(sys.argv[1])
totalNodeNumber = int(sys.argv[2])

def_ip = "127.0.0.1"
def_port = 25570

config = ET.Element("config")
ex_mode = ET.SubElement(config, "execution_mode").text = "1"
passwd = ET.SubElement(config, "passwd_sha256").text = "64E8DF328D36F9688A4AD76208CE3FC06AC582552531FF968742645C43DF1930"

node_self = ET.SubElement(config, "node_self")

idd = ET.SubElement(node_self, "id").text = str(ident)
ip = ET.SubElement(node_self, "ip").text = def_ip
port = ET.SubElement(node_self, "port").text = str(def_port + ident - 1)
pub = ET.SubElement(node_self, "pub").text = "../RSA_keys/RSA_pub" + str(ident) + ".der"
prv = ET.SubElement(node_self, "prv").text = "../RSA_keys/RSA_prv" + str(ident) + ".der"

network = ET.SubElement(config, "network")

for i in range(totalNodeNumber):
    i = i+1
    if (i != ident):
        node = ET.SubElement(network, "node")
        idd = ET.SubElement(node, "id").text = str(i)
        ip = ET.SubElement(node, "ip").text = def_ip
        port = ET.SubElement(node, "port").text = str(def_port + i - 1)
        pub = ET.SubElement(node, "pub").text = "../RSA_keys/RSA_pub" + str(i) + ".der"

tree = ET.ElementTree(config)
path = "../XML_config/config" + str(ident) + ".xml"
tree.write(path)
