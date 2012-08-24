#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

usage() {
    echo "$0 options"
    echo "$test_description"
    echo ""
    echo "Options:"
    echo "  -n              space separated nodes where test is executed"

    exit 1
}

set_no_nodes() {
    no_nodes=0
    for i in $nodes;do
	no_nodes=$(($no_nodes + 1))
    done
}

err() {
    echo "$*" >&2
}

test_required_nodes=${test_required_nodes:-1}
test_max_nodes=${test_max_nodes:-1}
test_max_runtime=${test_max_runtime:-300}
test_description=${test_description:-Test has no description}

while getopts "hn:" optflag; do
    case "$optflag" in
    h)
        usage
        ;;
    n)
        nodes="$OPTARG"
        ;;
    \?|:)
        usage
        ;;
    esac
done

set_no_nodes

if [ "$no_nodes" == 0 ] ||
  ([ "$test_required_nodes" != -1 ] && [ "$no_nodes" -lt "$test_required_nodes" ]);then
    err "Not enough nodes for test!"
    usage
fi

if [ "$test_max_nodes" != -1 ] && [ "$no_nodes" -gt "$test_max_nodes" ];then
    err "Too much nodes for test!"
    usage
fi

set -x -e
