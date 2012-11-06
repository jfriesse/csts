#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that all man pages have valid links"

. common.sh

#cat_mp manual_page
cat_mp() {
    local mp="$1"
    local cat_cmd="cat"

    if [ "${page_path%%*.gz}" != "$page_path" ] || [ "${page_path%%*.z}" != "$page_path" ] || \
      [ "${page_path%%*.Z}" != "$page_path" ];then
        cat_cmd="zcat"
    fi

    if [ "${page_path%%*.bz2}" != "$page_path" ];then
        cat_cmd="bzcat"
    fi

    if [ "${page_path%%*.xz}" != "$page_path" ];then
        cat_cmd="xzcat"
    fi

    run "$nodes_ip" "$cat_cmd \"$mp\""
}

alread_processed_pages=""

process_page() {
    local mp="$1"
    local sect="$2"
    local depth="${3:-1}"
    local page_path
    local mp_file
    local links
    local l

    if echo "$already_processed_pages" | grep " $mp.$sect " &>/dev/null;then
        return 0
    fi

    already_processed_pages="$already_processed_pages $mp.$sect "
    mp_file=`run "$nodes_ip" man -w "$sect" "$mp"`
    [ "$mp_file" == "" ] && return $depth
    echo "$mp.$sect"

    links=`cat_mp "$mp_file" | grep '^.BR ' | sed 's/^.BR \(.*\) (\(.*\)).*$/\1.\2/'`
    for l in $links;do
        process_page "${l%%.[0-9]}" "${l##*.}" "$(($depth + 1))" || return $?
    done
}

process_page "corosync" "8"
process_page "corosync-blackbox" "8"
process_page "corosync-cfgtool" "8"
process_page "corosync-cpgtool" "8"
process_page "corosync-notifyd" "8"

res=0
process_page "corosync-xmlproc" "8" || res=$?
if [ "$res" -gt 1 ];then
    exit 1
fi

echo "Processed" `echo "$already_processed_pages" | wc -w` "pages"
