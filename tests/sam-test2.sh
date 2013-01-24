#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test SAM - test recovery policy quit and signal delivery"

. inc/common.sh

compile_app "$nodes_ip" "sam-test2" "-lsam"
run_app "$nodes_ip" 'sam-test2'

exit 0
