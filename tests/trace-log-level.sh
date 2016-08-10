#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that trace level works"

. inc/common.sh

generate_corosync_conf_cb() {
    sed 's/^[ \t]*debug: .*$/debug: on/'
}

generate_corosync_conf_trace_cb() {
    sed 's/^[ \t]*debug: .*$/debug: trace/'
}

generate_corosync_conf_subsec_cb() {
    sed 's/^[ \t]*debug: .*$/debug: on/' | sed 's/^[ \t]*subsys: .*$/subsys: '$subsys'/' |
	sed '/subsys: /,$s/^[ \t]*debug: .*$/debug: trace/'
}

send_cpg_msg() {
    echo -e "sync\nexit\n" | run_app "$nodes_ip" 'cpg-cli-client'
}


is_msg_in_log() {
    # Give some time for logged entry to appear
    sleep 2
    cat_corosync_log "$nodes_ip" | grep "$msg_to_find"
}

assert_no_msg_debug_on() {
    # We should not have "got mcast request on" message if debug is: on
    configure_corosync "$nodes_ip"
    start_corosync "$nodes_ip"
    send_cpg_msg
    is_msg_in_log && exit 1
    stop_corosync "$nodes_ip"
}

assert_msg_debug_trace() {
    # We should get "got mcast request on" message if debug is: trace
    configure_corosync "$nodes_ip" "generate_corosync_conf_trace_cb"
    start_corosync "$nodes_ip"
    send_cpg_msg
    is_msg_in_log
    stop_corosync "$nodes_ip"
}

assert_msg_debug_dynamic() {
    # Test that we are able to change levels at runtime
    configure_corosync "$nodes_ip"
    start_corosync "$nodes_ip"
    send_cpg_msg
    is_msg_in_log && exit 1

    cmap_set "$nodes_ip" "logging.debug" "str" "trace"
    send_cpg_msg
    is_msg_in_log
    stop_corosync "$nodes_ip"
}

assert_msg_debug_subsys_trace() {
    # We should get "got mcast request on" message if debug is: trace for subsystem
    configure_corosync "$nodes_ip" "generate_corosync_conf_subsec_cb"
    start_corosync "$nodes_ip"
    send_cpg_msg
    is_msg_in_log
    stop_corosync "$nodes_ip"
}

compile_app "$nodes_ip" "cpg-cli-client" "-lcpg"

for ((i=0; i<2; i++));do
    if [ "$i" == 0 ];then
	msg_to_find='CPG.*got mcast request on '
	subsys="CPG"
    else
	msg_to_find='TOTEM.*mcasted message added to pending queue'
	subsys="TOTEM"
    fi

    echo "msg_to_find: $msg_to_find"
    echo "subsys: $subsys"

    assert_no_msg_debug_on
    assert_msg_debug_trace
    assert_msg_debug_dynamic
    assert_msg_debug_subsys_trace
done

exit 0
