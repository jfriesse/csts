#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test SAM - smoke test - restart process when it dies"

. inc/common.sh

compile_app "$nodes_ip" "sam-test3" "-lsam"
run_app "$nodes_ip" 'sam-test3'

exit 0
