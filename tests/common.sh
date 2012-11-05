#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

set -e

usage() {
    echo "$0 options"
    echo "$test_description"
    echo -n "This test needs "
    if [ $test_required_nodes == $test_max_nodes ];then
	echo -n "exactly $test_required_nodes"
    elif [ "$test_max_nodes" != -1 ];then
	echo -n "from $test_required_nodes up to $test_max_nodes"
    else
	echo -n "at least $test_required_nodes"
    fi
    echo " node(s) specified"
    echo ""
    echo "Options:"
    echo "  -n              space separated nodes where test is executed"
    echo "  -i              print parseable test informations"

    exit 1
}

set_no_nodes() {
    no_nodes=0
    for i in $nodes;do
	no_nodes=$(($no_nodes + 1))
    done
}

resolve_ip() {
    ping -q -c 1 -t 1 "$1" | grep PING | sed -e "s/.* (//" | sed -e "s/).*//"
}

resolve_nodes_ip() {
    nodes_ip=""
    for i in $nodes;do
	ip=`resolve_ip $i`
	[ "$ip" == "" ] && exit 1
	[ "$nodes_ip" != "" ] && nodes_ip="$nodes_ip "
	nodes_ip="$nodes_ip$ip"
    done
}

err() {
    echo "$*" >&2
}

prepare_node_dirs() {
    for i in $nodes;do
        ssh "root@$i" "mkdir -p $test_apps_dir $test_var_dir"
    done
}

run() {
    local node="$1"

    shift
    ssh "root@$node" "$*"
}

run_as() {
    local node="$1"
    local user="$2"

    shift 2
    run "$node" "su -s /bin/bash - $user -c '$*'"
}

compile_app() {
    local node="$1"
    local app="$2"
    local libs="$3"

    scp "../apps/$app.c" "root@$node:$test_apps_dir"
    run "$node" "cc $test_apps_dir/$app.c $libs -o $test_apps_dir/$app"
}

run_app() {
    local node="$1"
    local app="$2"
    shift 2
    local params="$*"

    run "$node" "cd $test_apps_dir; ./$app $params"
}

generate_corosync_conf_cb() {
    cat
}

# generate_corosync_conf node, [callback]
generate_corosync_conf() {
    local node="$1"
    local cb=generate_corosync_conf_cb

    [ "$2" != "" ] && cb="$2"

    sed '../configs/corosync.conf.example' -e 's/^[ \t]*bindnetaddr:.*$/    bindnetaddr: '$node'/' \
      -e 's/^[ \t]*mcastaddr:.*$/    mcastaddr: '$mcast_addr'/' | $cb | \
      run "$node" 'tee /var/csts/corosync.conf.used > /etc/corosync/corosync.conf'
}

# configure_corosync node, [callback]
configure_corosync() {
    local node="$1"

    if run "$node" "[ -f /etc/corosync/corosync.conf ]";then
	run "$node" "mv /etc/corosync/corosync.conf $test_var_dir/corosync.conf.bck"
    fi

    generate_corosync_conf "$node" "$2"
}

start_corosync() {
    local node="$1"
    local no_retries=0
    local probe_command

    run "$node" 'echo --- MARKER --- '$0' at `date +"%F-%T"` --- MARKER --- >> /var/log/cluster/corosync.log'
    run "$node" "corosync" || return $?

# Doesn't work for flatiron
#    probe_command='corosync-cfgtool -s > /dev/null 2>&1'
    probe_command='corosync-cpgtool > /dev/null 2>&1'

    while ! run "$node" "$probe_command" && [ $no_retries -lt 20 ]; do
        sleep 0.5
        no_retries=$(($no_retries + 1))
    done

    [ "$no_retries" -lt 20 ] && return 0 || return 1
}

stop_corosync() {
    local node="$1"
    local no_retries=0

    run "$node" 'kill -INT `cat /var/run/corosync.pid`'

    while ! cat_corosync_log "$node" | grep 'Corosync Cluster Engine exiting' && [ $no_retries -lt 20 ];do
	sleep 1
	no_retries=$(($no_retries + 1))
    done

    [ "$no_retries" -lt 20 ] && return 0 || return 1
}

kill_corosync() {
    local node="$1"

    run "$node" 'killall -9 corosync'
}

corosync_mem_used() {
    local node="$1"

    run "$node" 'ps -o rss -p `cat /var/run/corosync.pid` | sed -n 2p'
}

exit_trap_start_cb() {
    return 0
}

exit_trap_end_cb() {
    return 0
}

exit_trap() {
    [ "$alarm_pid" != "" ] && kill -INT $alarm_pid

    exit_trap_start_cb

    for i in $nodes_ip;do
	if run "$i" "[ -f /var/run/corosync.pid ]";then
	    stop_corosync "$i" || kill_corosync "$i" || true
	fi
	if run "$i" "[ -f $test_var_dir/corosync.conf.bck ]";then
	    run "$i" "mv -f $test_var_dir/corosync.conf.bck /etc/corosync/corosync.conf"
	fi
    done

    exit_trap_end_cb

    pkill -P $test_pid
}

cat_corosync_log() {
    local node="$1"

    run "$node" 'tac /var/log/cluster/corosync.log | sed -n -e "0,/^--- MARKER --- .* --- MARKER ---$/p" | tac |' \
	'sed 1d'
}

cmap_set() {
    local node="$1"
    local key="$2"
    local type="$3"
    local value="$4"

    if ! run "$node" "which corosync-cmapctl";then
	# Use corosync-objctl
	run "$node" "corosync-objctl -w $key=$value"
    else
	run "$node" "corosync-cmapctl -s $key $type $value"
    fi
}

cmap_get() {
    local node="$1"
    local key="$2"

    if ! run "$node" "which corosync-cmapctl";then
	# Use corosync-objctl
	run "$node" "corosync-objctl -a | grep '^$key=' | sed 's/^.*=//'"
    else
	run "$node" "corosync-cmapctl $key | sed 's/^.* = //'"
    fi
}

test_required_nodes=${test_required_nodes:-1}
test_max_nodes=${test_max_nodes:-1}
test_max_runtime=${test_max_runtime:-300}
test_description=${test_description:-Test has no description}
test_apps_dir="~/csts-apps"
test_var_dir="/var/csts"
corosync_running=0

while getopts "hin:" optflag; do
    case "$optflag" in
    h)
        usage
        ;;
    n)
        nodes="$OPTARG"
        ;;
    i)
	echo "test_required_nodes=$test_required_nodes"
	echo "test_max_nodes=$test_max_nodes"
	echo "test_max_runtime=$test_max_runtime"
	echo "test_description=$test_description"
	exit 0
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

resolve_nodes_ip

master_node=`echo $nodes_ip | cut -d ' ' -f 1`
mcast_addr="239.255."`echo $master_node | cut -d '.' -f 3-4`
test_pid=$$

prepare_node_dirs

trap 'exit_trap' EXIT

# Start alarm
(sleep $test_max_runtime; err "Test took too long"; while true;do kill -ALRM $test_pid; sleep 10;done) &
alarm_pid=$!

set -x -e
