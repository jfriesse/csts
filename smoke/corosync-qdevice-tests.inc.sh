##################
# Test functions #
##################
test_corosync_qdevice_h() {
    # Check that corosync-qdevice(-tool) binary exists and -h returns help text
    res=`corosync-qdevice -h || true`
    [ "$res" != "${res/usage/}" ]
    res=`corosync-qdevice-tool -h || true`
    [ "$res" != "${res/usage/}" ]
}

test_corosync_qnetd_h() {
    # Check that corosync-qnetd(-tool) binary exists and -h returns help text
    res=`corosync-qnetd -h || true`
    [ "$res" != "${res/usage/}" ]
    res=`corosync-qnetd-tool -h || true`
    [ "$res" != "${res/usage/}" ]
}

test_crt_creation() {
    # Erase old certificates
    rm -rf "$COROSYNC_SYSCONFD/qdevice"
    rm -rf "$COROSYNC_SYSCONFD/qnetd"

    corosync-qnetd-certutil -i
    corosync-qdevice-net-certutil -i -c "$COROSYNC_SYSCONFD/qnetd/nssdb/qnetd-cacert.crt"
    corosync-qdevice-net-certutil -r -n "$COROSYNC_CLUSTER_NAME"
    corosync-qnetd-certutil -s -c "$COROSYNC_SYSCONFD/qdevice/net/nssdb/qdevice-net-node.crq" -n "$COROSYNC_CLUSTER_NAME"
    corosync-qdevice-net-certutil -M -c "$COROSYNC_SYSCONFD/qnetd/nssdb/cluster-$COROSYNC_CLUSTER_NAME.crt"
}

test_qnetd_start() {
    service_start "corosync-qnetd"
}

test_qdevice_start() {
    service_start "corosync-qdevice"
}

test_corosync_start() {
    generate_corosync_conf "off" "" "on" > "$COROSYNC_CONF"
    cat "$COROSYNC_CONF"

    service_start "corosync"
}

test_qdevice_stop() {
    service_stop "corosync-qdevice"
}

test_qnetd_stop() {
    service_stop "corosync-qnetd"
}

test_corosync_stop() {
    service_stop "corosync"
}

########
# main #
########
if [ -z "$PREFIX" ];then
    echo "PREFIX not defined. Do not run *.inc.sh directly"
    exit 1
fi

test_corosync_qdevice_h
test_corosync_qnetd_h

#test_crt_creation

test_qnetd_start
test_corosync_start
test_qdevice_start

test_qdevice_stop
test_corosync_stop
test_qnetd_stop
