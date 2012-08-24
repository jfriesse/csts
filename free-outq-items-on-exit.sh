#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that outq items are freed on exit of IPC connection."

. common.sh

compile_app "$nodes" "free-outq-items-on-exit" "-lcpg"
run_app "$nodes" "free-outq-items-on-exit"

start_corosync
