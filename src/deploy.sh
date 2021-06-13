#!/bin/sh
#mkdir RSA_keys
#mkdir XML_config

node_number=$1
generate_keys_and_confs=$2

if [ $node_number -lt '4' ]; then
    echo "Error, enter valid node number (> 4) and generator g (optional) \n";
    exit 1
fi

if [ -z "$generate_keys_and_confs" ]; then
    echo "Deploting network locally \n"
    for i in $(seq 1 $node_number)
        do
            gnome-terminal -x sh -c "./run $i ; bash"
        done
else
    echo "Generating keys and configurations... \n"
    for i in $(seq 1 $node_number)
        do
            ../deploy_utils/generate_keys $i
            python3 ../deploy_utils/generate_configs.py $i $1
        done
fi