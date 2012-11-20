#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test SAM - test recovery policy restart"

. common.sh

compile_app "$nodes_ip" "sam-test1" "-lsam"
run_app "$nodes_ip" 'sam-test1'

exit 0
