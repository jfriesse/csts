#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

# fw_block_udp nodes
fw_block_udp() {
    local nodes="$1"
    local node

    for node in $nodes;do
        run "$node" "iptables -A INPUT ! -i lo -p udp -j DROP && iptables -A OUTPUT ! -o lo -p udp -j DROP" || return $?
    done

    return 0
}

# fw_unblock_udp nodes ignore_failure
fw_unblock_udp() {
    local nodes="$1"
    local ignore_failure="$2"
    local node

    for node in $nodes;do
        if [ "$ignore_failure" == "true" ];then
            run "$node" "iptables -D INPUT ! -i lo -p udp -j DROP || true; iptables -D OUTPUT ! -o lo -p udp -j DROP || true" || return $?
        else
            run "$node" "iptables -D INPUT ! -i lo -p udp -j DROP && iptables -D OUTPUT ! -o lo -p udp -j DROP" || return $?
        fi
    done

    return 0
}
