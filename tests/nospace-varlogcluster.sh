#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that full /var/log/cluster doesn't cause problem (blocking)"

. inc/common.sh
. inc/cpg-load.sh

nospace_image_file_size=1000
nospace_image_mount_point="/var/log/cluster"

. inc/nospace.sh

generate_corosync_conf_cb() {
    sed 's/^[ \t]*debug: .*$/debug: trace/'
}

configure_corosync "$nodes_ip"
cpg_load_prepare "$nodes_ip"
nospace_init "$nodes_ip"

start_corosync "$nodes_ip"

nospace_fill "$nodes_ip"
cpg_load_one_shot "$nodes_ip" "10000"

nospace_flush "$nodes_ip"

stop_corosync "$nodes_ip"

exit 0
