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
    echo "  -c              corosync version (flatiron|needle)"
    echo "  -v              use valgrind for corosync start"

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
	nodes_ip_array=( "${nodes_ip_array[@]}" "$ip" )
    done
}

err() {
    echo "$*" >&2
}

prepare_node_dirs() {
    for i in $nodes;do
        ssh "root@$i" "mkdir -p $test_apps_dir $test_var_dir"
    done

    mkdir -p "$test_var_dir"
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

    if run "$node" "[ -f /etc/corosync/corosync.conf ]" && ! run "$node" "[ -f $test_var_dir/corosync.conf.bck ]";then
	run "$node" "mv /etc/corosync/corosync.conf $test_var_dir/corosync.conf.bck"
    fi

    generate_corosync_conf "$node" "$2"
}

start_corosync_insert_marker() {
    local node="$1"

    run "$node" 'echo --- MARKER --- '$0' at `date +"%F-%T"` --- MARKER --- >> /var/log/cluster/corosync.log'
}

start_corosync_wait_for_start() {
    local node="$1"
    local no_retries=0
    local probe_command

# Doesn't work for flatiron
#    probe_command='corosync-cfgtool -s > /dev/null 2>&1'
    probe_command='corosync-cpgtool > /dev/null 2>&1'

    while ! run "$node" "$probe_command" && [ $no_retries -lt 20 ]; do
        sleep 0.5
        no_retries=$(($no_retries + 1))
    done

    [ "$no_retries" -lt 20 ] && return 0 || return 1
}

start_corosync() {
    local node="$1"

    start_corosync_insert_marker "$node"

    if $use_valgrind;then
        run "$node" "nohup valgrind corosync -f &> /var/log/csts-vg-corosync.log & echo" || return $?
    else
        run "$node" "corosync" || return $?
    fi

    start_corosync_wait_for_start "$node" || return $?

    return 0
}

# cat_valgrind_log [delete_after_cat]
cat_valgrind_log() {
    if run "$node" "[ -f /var/log/csts-vg-corosync.log ]";then
	run "$node" "cat /var/log/csts-vg-corosync.log"
	if [ "$1" == true ];then
	    run "$node" "rm -f /var/log/csts-vg-corosync.log"
	fi
    fi
}

# stop_corosync [use_corosync-cfgtool]
stop_corosync() {
    local node="$1"
    local use_cfgtool="$2"
    local no_retries=0

    if [ "$use_cfgtool" == true ];then
	run "$node" 'corosync-cfgtool -H'
    else
        run "$node" 'kill -INT `cat /var/run/corosync.pid`'
    fi

    while ! cat_corosync_log "$node" | grep 'Corosync Cluster Engine exiting' && [ $no_retries -lt 20 ];do
	sleep 1
	no_retries=$(($no_retries + 1))
    done

    if $use_valgrind;then
        cat_valgrind_log true
    fi

    if [ "$no_retries" -lt 20 ];then
	return 0
    else
        if ! run "$node" "[ -f /var/run/corosync.pid ]" && ! run "$node" "pgrep -f corosync &>/dev/null";then
            return 0
        else
            return 1
        fi
    fi
}

kill_corosync() {
    local node="$1"

    if $use_valgrind;then
        run "$node" 'pkill -f -9 corosync'
        cat_valgrind_log true
    else
        run "$node" 'killall -9 corosync'
    fi
}

corosync_mem_used() {
    local node="$1"

    run "$node" 'ps -o rss -p `cat /var/run/corosync.pid` | sed -n 2p'
}

# pause_corosync node
pause_corosync() {
    local node="$1"

    run "$node" 'kill -STOP `cat /var/run/corosync.pid`'
}

# unpause_corosync node
unpause_corosync() {
    local node="$1"

    run "$node" 'kill -CONT `cat /var/run/corosync.pid`'
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

	if [ "$confchg_used" == "true" ];then
            confchg_stop "$i"
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

    if ! run "$node" "which corosync-cmapctl &>/dev/null";then
	# Use corosync-objctl
	run "$node" "corosync-objctl -w $key=$value"
    else
	run "$node" "corosync-cmapctl -s $key $type $value"
    fi
}

cmap_get() {
    local node="$1"
    local key="$2"

    if ! run "$node" "which corosync-cmapctl &>/dev/null";then
	# Use corosync-objctl
	run "$node" "corosync-objctl -a | grep '^$key=' | sed 's/^.*=//'"
    else
	run "$node" "corosync-cmapctl $key | sed 's/^.* = //'"
    fi
}

# Similar like wait, but returns error code if ANY of processes returned nonzero status code
strict_wait() {
    local res=0

    for pid in $*;do
        wait $pid || res=$?
    done

    return $res
}

randomize_word_order() {
    local words=($*)
    local no_words
    local item_pos
    local item

    while [ "${#words[@]}" != 0 ];do
        no_words=${#words[@]}
        item_pos=$(($RANDOM % $no_words))
        item=${words[$item_pos]}
        unset words[$item_pos]
        words=(${words[@]})
        echo -n "$item "
    done

    return 0
}

test_required_nodes=${test_required_nodes:-1}
test_max_nodes=${test_max_nodes:-1}
test_max_runtime=${test_max_runtime:-300}
test_description=${test_description:-Test has no description}
test_corover_flatiron_enabled=${test_corover_flatiron_enabled:-true}
[ "$test_corover_needle_enabled" == "" ] && test_corover_needle_enabled=$test_corover_flatiron_enabled
if [ "$test_corover_undefined_enabled" == "" ];then
    test_corover_undefined_enabled=false
    "$test_corover_flatiron_enabled" && "$test_corover_needle_enabled" && test_corover_undefined_enabled=true
fi

test_apps_dir="~/csts-apps"
test_var_dir="/var/csts"
corosync_running=0
corosync_version="undefined"
use_valgrind=false

while getopts "hivc:n:" optflag; do
    case "$optflag" in
    h)
        usage
        ;;
    n)
        nodes="$OPTARG"
        ;;
    c)
	corosync_version="$OPTARG"
	;;
    i)
	echo "test_required_nodes=$test_required_nodes"
	echo "test_max_nodes=$test_max_nodes"
	echo "test_max_runtime=$test_max_runtime"
	echo "test_description=$test_description"
	echo "test_corover_undefined_enabled=$test_corover_undefined_enabled"
	echo "test_corover_flatiron_enabled=$test_corover_flatiron_enabled"
	echo "test_corover_needle_enabled=$test_corover_needle_enabled"
	exit 0
	;;
    v)
        use_valgrind=true
        ;;
    \?|:)
        usage
        ;;
    esac
done

case "$corosync_version" in
"flatiron")
    if ! $test_corover_flatiron_enabled;then
        err "Test doesn't support corosync flatiron"
        usage
    fi
    ;;
"needle")
    if ! $test_corover_needle_enabled;then
        err "Test doesn't support corosync needle"
        usage
    fi
    ;;
"undefined")
    if ! $test_corover_undefined_enabled;then
        err "Test doesn't support undefined version of corosync"
        usage
    fi
    ;;
*)
    err "Unsupported corosync version"
    usage
    ;;
esac

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

master_node=${nodes_ip_array[0]}
mcast_addr="239.255."`echo $master_node | cut -d '.' -f 3-4`
test_pid=$$

prepare_node_dirs

trap 'exit_trap' EXIT

# Start alarm
(sleep $test_max_runtime && err "Test took too long" && while true;do kill -ALRM $test_pid; sleep 10;done) &
alarm_pid=$!

set -x -e
