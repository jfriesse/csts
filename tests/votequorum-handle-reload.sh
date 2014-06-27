#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that votequorum handles reload (registers reload_in_progress)"
test_corover_flatiron_enabled=false
test_corover_needle_enabled=true

. inc/common.sh
. inc/token-set.sh

generate_corosync_conf_cb() {
    cat
}

# generate_nodelist_config no_nodes
store_corosync_nodelist_config() {
    local no_nodes="$1"

    new_config=`mktemp`
    cat "$original_config" > "$new_config"

    echo "quorum {" >> "$new_config"
    echo "  provider: corosync_votequorum" >> "$new_config"
    echo "}" >> "$new_config"

    echo "nodelist {" >> "$new_config"

    my_node=${nodes_ip##*.}

    for ((node=1; node<$no_nodes; node++)) {
        if [ "$node" != "$my_node" ];then
            echo "	node {" >> "$new_config"
            echo "		ring0_addr: "${master_node%.*}".$node" >> "$new_config"
            echo "	}" >> "$new_config"
        fi
    }

    echo "	node {" >> "$new_config"
    echo "		ring0_addr: "${master_node%.*}".$my_node" >> "$new_config"
    echo "	}" >> "$new_config"

    echo "}" >> "$new_config"

    cat "$new_config" | store_corosync_config "$nodes_ip"
    rm -f "$new_config"
}

configure_corosync "$nodes_ip"

original_config=`mktemp`
get_current_corosync_config "$nodes_ip" > "$original_config"
store_corosync_nodelist_config 10

start_corosync "$nodes_ip"

store_corosync_nodelist_config 1
run "$nodes_ip" "corosync-cfgtool -R"

(cat_corosync_log "$nodes_ip" | grep 'configuration error: nodelist or .*expected_votes must be configured!') && exit 1

stop_corosync "$nodes_ip"

exit 0
