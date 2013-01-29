#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test cpg - pause node, send message, unpause results in message delivered to both nodes after confchg"
test_required_nodes=2
test_max_nodes=2

. inc/common.sh

#wait_for_msg msg [redeliver]
wait_for_msg() {
    local msg="$1"
    local redeliver="$2"
    local all_delivered=false
    local no_retries=0

    while [ $no_retries -lt 40 ] && [ "$all_delivered" == false ];do
        all_delivered=true

        for node in $nodes_ip;do
            if ! run "$node" "cat /tmp/testcpg.log" | grep "^DeliverCallback: message.*'$msg\$" &>/dev/null;then
                all_delivered=false
            fi
        done

        if [ "$all_delivered" == "false" ];then
            sleep 0.5
            no_retries=$(($no_retries + 1))
            if [ "$redeliver" == true ];then
                echo "$msg" >&60
            fi
        fi
    done

    [ "$all_delivered" == true ] && return 0 || return 1
}

for node in $nodes_ip;do
    configure_corosync "$node"
    start_corosync "$node"
    compile_app "$node" "testcpg" "-lcpg"
done

for ((i=0; i<$no_nodes; i++));do
    for ((j=0; j<$no_nodes; j++));do
        eval `echo 'exec '$((60+$j))'> >(run_app "${nodes_ip_array[$j]}" "testcpg" ">/tmp/testcpg.log")'`
    done

    wait_for_msg "INIT PING" true
    pause_corosync "${nodes_ip_array[$i]}"

    # Wait for corosync to really pause
    sleep 5

    eval `echo 'echo "PAUSE PONG" >&'$((60+$i))`
    unpause_corosync "${nodes_ip_array[$i]}"
    wait_for_msg "PAUSE PONG" false

    # Close file descriptors
    for ((j=0; j<$no_nodes; j++));do
        eval `echo 'exec '$((60+$j))'>&-'`
    done
done

exit 0
