#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that totem key changes properly handles dependencies"
test_corover_flatiron_enabled=false
test_corover_needle_enabled=true

. inc/common.sh
. inc/token-set.sh

# generate_nodelist_config no_nodes
store_corosync_nodelist_config() {
    new_config=`mktemp`
    cat "$original_config" > "$new_config"
    echo "nodelist {" >> "$new_config"

    for ((node=1; node<=$no_nodes; node++)) {
        echo "	node {" >> "$new_config"
        echo "		ring0_addr: "${master_node%.*}".$node" >> "$new_config"
        echo "	}" >> "$new_config"
    }

    echo "}" >> "$new_config"

    cat "$new_config" | store_corosync_config "$nodes_ip"
    rm -f "$new_config"
}

max_no_nodes=10

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

original_config=`mktemp`
get_current_corosync_config "$nodes_ip" > "$original_config"

# First check if adding nodes to config file increase timeout
token_inc[0]=0
consensus_inc[0]=0
for ((no_nodes=1; no_nodes<=$max_no_nodes; no_nodes++)) {
    store_corosync_nodelist_config "$no_nodes"
    run "$nodes_ip" "corosync-cfgtool -R"
    token_inc[$no_nodes]=`receive_current_token_timeout`
    consensus_inc[$no_nodes]=`receive_current_consensus_timeout`

    [ ${consensus_inc[$no_nodes]} -ge ${consensus_inc[$no_nodes-1]} ]
    [ ${token_inc[$no_nodes]} -ge ${token_inc[$no_nodes-1]} ]
}

# Check if token|consensus was really increased
for ((i=1; i<max_no_nodes/2;i++));do
    [ ${consensus_inc[$max_no_nodes]} -gt ${consensus_inc[$i]} ]
    [ ${token_inc[$max_no_nodes]} -gt ${token_inc[$i]} ]
done

# Now try the same with feature disabled - consensus should be smaller now
for method in cmap config cmap config;do
    change_tc_value "token_coefficient" "0" "$method"

    if [ "$method" == "config" ];then
        run "$nodes_ip" "corosync-cfgtool -R"
    fi

    [ `receive_current_token_timeout` -lt ${token_inc[$max_no_nodes]} ]
    [ `receive_current_consensus_timeout` -lt ${consensus_inc[$max_no_nodes]} ]
    change_tc_value "token_coefficient" "del" "$method"
    if [ "$method" == "config" ];then
        run "$nodes_ip" "corosync-cfgtool -R"
    fi
done

# Now try decrease number of nodes
token_dec[0]=0
consensus_dec[0]=0
for ((no_nodes=$max_no_nodes; no_nodes>0; no_nodes--)) {
    store_corosync_nodelist_config "$no_nodes"
    run "$nodes_ip" "corosync-cfgtool -R"
    token_dec[$no_nodes]=`receive_current_token_timeout`
    consensus_dec[$no_nodes]=`receive_current_consensus_timeout`
}

# And check if consensus|token_inc == consensus|token_dec
[ "${token_inc[*]}" == "${token_dec[*]}" ]
[ "${consensus_inc[*]}" == "${consensus_dec[*]}" ]

stop_corosync "$nodes_ip"

exit 0
