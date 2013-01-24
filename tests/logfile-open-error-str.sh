#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that error message is properly saved"

. inc/common.sh

generate_corosync_conf_cb() {
    sed 's/^[ \t]*logfile: .*$/logfile: \/nonexistingdir\/corosync.log/'
}

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip" && exit 1
start_corosync "$nodes_ip" 2>&1 | grep -i 'no such file or directory'

exit 0
