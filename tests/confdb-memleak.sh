#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test confdb - mem leak"

. inc/common.sh

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

if run "$nodes_ip" 'cat /usr/include/corosync/confdb.h' &>/dev/null;then
    app_name="confdb-memleak"
    compile_params="-lconfdb"
else
    app_name="cmap-memleak"
    compile_params="-lcmap"
fi

compile_app "$nodes_ip" "$app_name" "$compile_params"
run_app "$nodes_ip" "$app_name -1d"
mem_used_start=`corosync_mem_used "$nodes_ip"`
run_app "$nodes_ip" "$app_name -2d"
mem_used_end=`corosync_mem_used "$nodes_ip"`

[ $mem_used_end -gt $(($mem_used_start / 10 + $mem_used_start)) ] && exit 1

exit 0
