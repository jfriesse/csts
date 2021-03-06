#!/bin/bash

set -e

. inc/tar-writer.sh

usage() {
    echo "$0 options"
    echo "Runs tests in tests directory"
    echo ""
    echo "Options:"
    echo "  -n              space separated nodes where test is executed"
    echo "  -l logfile      write complete log to given logfile (stdout by default)"
    echo "  -c              corosync version (flatiron|needle)"

    exit 1
}

# 1 - No nodes to find
# Returns nodes number to use or nothing if there is not enough nodes
find_free_nodes() {
    ret=""
    no_found=0

    for ((i = 0; i < ${#test_nodes[*]}; i++));do
	if [ "${test_nodes_pid[$i]}" == 0 ];then
	    ret="$ret $i"
	    no_found=$(($no_found + 1))
	    if [ "$no_found" == "$1" ];then
		echo $ret
		return 0
	    fi
	fi
    done
}

run_test() {
    local test_name=$1
    local test_out=$2
    local node_names=$3

    echo "---------- $test_name on $node_names ----------"

    res=0
    "./$test_name" -c "$corosync_version" -n "$node_names" || res=$?

    echo "---------- $test_name result = $res ----------"

    exit $res
}

# 1 - test to start
# 2 - nodes to use
start_test() {
    local test_out=`mktemp`
    local node_names=""
    local test_name=$1

    for i in $2;do
	[ "$node_names" != "" ] && node_names="$node_names "
	node_names="$node_names${test_nodes[$i]}"
    done

    run_test "$test_name" "$test_out" "$node_names" &> $test_out &
    tpid=$!

    j=0
    for i in $2;do
	test_nodes_pid[$i]=$tpid
	if [ "$j" == 0 ];then
	    test_nodes_test_out[$i]=$test_out
	    test_nodes_test_name[$i]="$test_name"
	    j=1
	fi
    done
}

# Wait for any child to exit
wait_for_any() {
    while true; do
	for pid in ${test_nodes_pid[*]}; do
	    if ! kill -0 "$pid" &> /dev/null;then
		echo $pid
		return 0
	    fi
    	done
    	sleep 1
    done
}

# 1 - pid
finish_test() {
    local res=0
    local pid=$1

    wait $pid || res=$?

    j=0
    for ((i = 0; i < ${#test_nodes[*]}; i++));do
	if [ ${test_nodes_pid[$i]} == "$pid" ];then
	    test_nodes_pid[$i]=0
	    if [ "$j" == 0 ];then
		test_out=${test_nodes_test_out[$i]}
		test_name=${test_nodes_test_name[$i]}
	        j=1
	    fi
	fi
    done

    if [ $res == 0 ];then
	echo "pass $test_name"
	no_pass=$(($no_pass + 1))
    else
	echo "FAILED $test_name"
	no_failed=$(($no_failed + 1))
    fi

    cat $test_out >> $complete_test_out
    echo >> $complete_test_out
    rm -f $test_out
}

wait_for_all_tests() {
    local need_wait=true

    while [ "$need_wait" == true ];do
	need_wait=false
	for ((i = 0; i < ${#test_nodes[*]}; i++));do
	    [ ${test_nodes_pid[$i]} != "0" ] && need_wait=true
        done

        if [ "$need_wait" == true ];then
	    dead_pid=`wait_for_any`
	    finish_test "$dead_pid"
        fi
    done
}

set_no_nodes() {
    no_nodes=0
    for i in $nodes;do
	no_nodes=$(($no_nodes + 1))
    done
}

init_test_nodes_pid() {
    for ((i = 0; i < $no_nodes; i++));do
	test_nodes_pid[$i]=0
    done
}

logfile="/dev/stdout"

while getopts "hic:l:n:" optflag; do
    case "$optflag" in
    h)
	usage
        ;;
    n)
        nodes="$OPTARG"
        ;;
    l)
	logfile="$OPTARG"
	;;
    c)
	corosync_version="$OPTARG"
	;;
    \?|:)
	usage
        ;;
    esac
done

if [ "$corosync_version" == "" ];then
    echo "corosync version must be specified"
    usage
fi

if [ "$corosync_version" != "flatiron" ] && [ "$corosync_version" != "needle" ];then
    echo "Unsupported corosync version"
    usage
fi

set_no_nodes

if [ "$no_nodes" -lt 1 ];then
    echo "There must be at least one node specified"
    usage
fi

read -ra test_nodes <<< "$nodes"
declare -a test_nodes_pid; init_test_nodes_pid
declare -a test_nodes_test_out
declare -a test_nodes_test_name

complete_test_out=`mktemp`
no_failed=0
no_pass=0
no_skipped=0

pushd tests &>/dev/null

for test_exec in *.sh;do
    [ ! -x "$test_exec" ] && continue
    test_info=`./$test_exec -i`
    nodes_needed=`echo "$test_info" | grep test_required_nodes | cut -d '=' -f 2-`
    corover_enabled=`echo "$test_info" | grep test_corover_${corosync_version}_enabled | cut -d '=' -f 2-`

    if [ "$nodes_needed" -gt "$no_nodes" ] || ! $corover_enabled;then
	echo "skipped $test_exec"
	no_skipped=$(($no_skipped + 1))
	continue
    fi

    free_nodes=""
    while [ "$free_nodes" == "" ];do
	free_nodes=`find_free_nodes $nodes_needed`
	if [ "$free_nodes" == "" ];then
	    dead_pid=`wait_for_any`
	    finish_test "$dead_pid"
	fi
    done

    # Start test on given nodes
    start_test "$test_exec" "$free_nodes"
done

wait_for_all_tests

popd &>/dev/null

echo
echo "FAILED: $no_failed, pass: $no_pass, skipped: $no_skipped, total: $(($no_pass + $no_failed))"
echo
cat $complete_test_out > $logfile
rm -f $complete_test_out

[ "$no_failed" == 0 ]
