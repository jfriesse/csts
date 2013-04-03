#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that parsing of uidgid entries works"
test_corover_undefined_enabled=false

. inc/common.sh

gen_conf() {
    cat
    echo "uidgid {"
    if [ "$conf_uid" != "" ];then
        echo "    uid: $conf_uid"
    fi
    if [ "$conf_gid" != "" ];then
        echo "    gid: $conf_gid"
    fi
    echo "}"
}

gen_conf_needle_error() {
    cat
    echo "uidgid {"
        echo "    something: nobody"
    echo "}"
}

# check_uidgid_exists uid|gid id
check_uidgid_exists() {
    if [ "$corosync_version" == "needle" ];then
        [ "`cmap_get \"$nodes_ip\" \"uidgid.$1.$2\"`" == "1" ]
    fi

    if [ "$corosync_version" == "flatiron" ];then
        [ "`cmap_get \"$nodes_ip\" \"uidgid.$1\"`" == "$2" ]
    fi
}

for ((i=0; i<2; i++));do
    conf_uid=""
    conf_gid=""

    case "$i" in
    "0")
        conf_uid="nonexistinguid"
        ;;
    "1")
        conf_gid="nonexistinggid"
        ;;
    esac

    configure_corosync "$nodes_ip" gen_conf
    start_corosync "$nodes_ip" && exit 1 || true
done

# Needle parser throws error if uidgid contains item other then uid|gid
if [ "$corosync_version" == "needle" ];then
    configure_corosync "$nodes_ip" gen_conf_needle_error
    start_corosync "$nodes_ip" && exit 1 || true
fi

for ((i=0; i<4; i++));do
    conf_uid=""
    conf_gid=""

    case "$i" in
    "0")
        conf_uid="nobody"
        res_uid=$conf_uid
        if [ "$corosync_version" == "needle" ];then
            res_uid=`run "$nodes_ip" "id -r -u $conf_uid"`
        fi
        ;;
    "1")
        conf_gid="nobody"
        res_gid=$conf_gid
        if [ "$corosync_version" == "needle" ];then
            res_gid=`run "$nodes_ip" "id -r -g $conf_gid"`
        fi
        ;;
    "2")
        conf_uid="99"
        res_uid=$conf_uid
        ;;
    "3")
        conf_gid="99"
        res_gid=$conf_gid
        ;;
    esac

    configure_corosync "$nodes_ip" gen_conf
    start_corosync "$nodes_ip"
    check_uidgid_exists `[ "$conf_uid" != "" ] && echo "uid" || echo "gid"` \
        `[ "$conf_uid" != "" ] && echo "$res_uid" || echo "$res_gid"`
    stop_corosync "$nodes_ip"
done

exit 0
