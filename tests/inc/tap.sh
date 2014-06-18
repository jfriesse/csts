#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

tap_delete_device() {
    local dev_name="$1"

    if run "$nodes_ip" "which tunctl" >/dev/null;then
        # Tunctl exists
        run "$nodes_ip" "tunctl -d $dev_name" || return $?
    else
        run "$nodes_ip" "ip tuntap del dev $dev_name mode tap" || return $?
    fi

    return 0
}

tap_create_device() {
    if run "$nodes_ip" "which tunctl" >/dev/null;then
        # Tunctl exists
        run "$nodes_ip" "tunctl -b -p"
    else
        # Check ip tuntap
        run "$nodes_ip" "ip tuntap" >/dev/null
        [ "$?" != 0 ] && return 1

        run "$nodes_ip" "ip tuntap add dev cststap0 mode tap" >/dev/null || return $?
        echo "cststap0"
    fi

    return 0
}
