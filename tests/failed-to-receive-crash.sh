#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that corosync doesn't crash on FAILED TO RECEIVE condition"
test_required_nodes=2
test_max_nodes=2
test_max_runtime=1200

. inc/common.sh

node_a=${nodes_ip% *}
node_b=${nodes_ip#* }

exit_trap_end_cb() {
    run "$node_a" "iptables -D INPUT ! -i lo -p udp -m limit --limit 10000/s --limit-burst 1 -j ACCEPT"
    run "$node_a" "iptables -D INPUT ! -i lo -p udp ! --sport domain -j DROP"
}

generate_corosync_conf_cb_node_a() {
    sed '/^[ \t]*crypto_hash:/a \
fail_recv_const: 5'
}


configure_corosync "$node_a" generate_corosync_conf_cb_node_a
configure_corosync "$node_b"

start_corosync "$node_a"
start_corosync "$node_b"

compile_app "$node_b" "cpg-cli-client" "-lcpg"

run "$node_a" "iptables -A INPUT ! -i lo -p udp -m limit --limit 10000/s --limit-burst 1 -j ACCEPT"
run "$node_a" "iptables -A INPUT ! -i lo -p udp ! --sport domain -j DROP"

echo "Sending 1000 messages"
echo -e "sendrandburst 1000 1 16\nsync\nexit" | run_app "$node_b" 'cpg-cli-client' > /dev/null

[ "`run $node_a 'pidof corosync'`" == "`run $node_a 'cat /var/run/corosync.pid'`" ] && \
    [ "`run $node_b 'pidof corosync'`" == "`run $node_b 'cat /var/run/corosync.pid'`" ]

exit 0
