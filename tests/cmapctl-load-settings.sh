#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test corosync-cmapctl -p option"
test_corover_flatiron_enabled=false
test_corover_needle_enabled=true

. inc/common.sh

exit_trap_end_cb() {
    rm -f "$orig_cmap" "$new_cmap" "$tmp1_cmap" "$tmp2_cmap" || true
    run "$nodes_ip" "rm -f $node_cmap" || true
}

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

# Prepare tmp files
orig_cmap=`mktemp`
node_cmap=`run "$nodes_ip" "mktemp"`
new_cmap=`mktemp`
tmp1_cmap=`mktemp`
tmp2_cmap=`mktemp`

# Store original cmap
run "$nodes_ip" "corosync-cmapctl" > "$orig_cmap"

# Try to add values
run "$nodes_ip" "cat > $node_cmap" << EOF
testkey.value1 u8 1
testkey.value2 u16 2
testkey.value3 u32 3
testkey.value4 u64 4
testkey.value5 i8 -1
testkey.value6 i16 2
testkey.value7 i32 -3
testkey.value8 i64 4
testkey.value9 flt 5.5
testkey.value10 dbl 6.6
testkey.value11 str Hello
EOF

run "$nodes_ip" "corosync-cmapctl -p $node_cmap"

# Check that values are really added
[ "`cmap_get \"$nodes_ip\" \"testkey.value1\"`" == "1" ]
[ "`cmap_get \"$nodes_ip\" \"testkey.value2\"`" == "2" ]
[ "`cmap_get \"$nodes_ip\" \"testkey.value3\"`" == "3" ]
[ "`cmap_get \"$nodes_ip\" \"testkey.value4\"`" == "4" ]
[ "`cmap_get \"$nodes_ip\" \"testkey.value5\"`" == "-1" ]
[ "`cmap_get \"$nodes_ip\" \"testkey.value6\"`" == "2" ]
[ "`cmap_get \"$nodes_ip\" \"testkey.value7\"`" == "-3" ]
[ "`cmap_get \"$nodes_ip\" \"testkey.value8\"`" == "4" ]
[[ "`cmap_get \"$nodes_ip\" \"testkey.value9\"`" =~ 5\.50*$ ]]
[[ "`cmap_get \"$nodes_ip\" \"testkey.value10\"`" =~ 6\.60*$ ]]
[ "`cmap_get \"$nodes_ip\" \"testkey.value11\"`" == "Hello" ]

# Now try delete operation
run "$nodes_ip" "cat > $node_cmap" << EOF
^^testkey.value1
testkey.value20 u8 1
^testkey.value3 u32 3
EOF

run "$nodes_ip" "corosync-cmapctl -p $node_cmap"

[ "`cmap_get \"$nodes_ip\" \"testkey.value1\"`" == "" ]
[ "`cmap_get \"$nodes_ip\" \"testkey.value10\"`" == "" ]
[ "`cmap_get \"$nodes_ip\" \"testkey.value11\"`" == "" ]
[ "`cmap_get \"$nodes_ip\" \"testkey.value3\"`" == "" ]
[ "`cmap_get \"$nodes_ip\" \"testkey.value20\"`" == "1" ]

exit 0
