#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

# Default path of image file (loop)
nospace_image_file=${nospace_image_file:-"$test_var_dir/imgfile.img"}
# Size of image (in kbytes)
nospace_image_file_size=${nospace_image_file_size:-0}
# Directory where to mount image to
nospace_image_mount_point=${nospace_image_mount_point:-""}
# File to used as fill file
nospace_fill_file="$nospace_image_mount_point/fill"

nospace_exit_trap_end_cb() {
    return 0
}

exit_trap_end_cb() {
    if run "$nodes_ip" mount | grep "$nospace_image_file";then
        run "$nodes_ip" umount "$nospace_image_file"
    fi

    nospace_exit_trap_end_cb
}

# nospace_init node
# Init nospace. Creates loop image, mounts it, ...
nospace_init() {
    local node="$1"

    [ "$nospace_image_file_size" -gt 0 ] || return $?
    [ "$nospace_image_mount_point" != "" ] || return $?

    run "$node" dd if=/dev/zero of="$nospace_image_file" bs=1024 count=$nospace_image_file_size || return $?
    run "$node" mkfs.ext2 -F "$nospace_image_file" || return $?
    run "$node" mount "$nospace_image_file" "$nospace_image_mount_point" -o loop || return $?

    return 0
}

# nospace_fini node
# Finalize nospace. Unmounts and deletes loop image
nospace_fini() {
    local node="$1"

    if run "$node" mount | grep "$nospace_image_file";then
        run "$node" umount "$nospace_image_file"

        run "$node" rm -f "$nospace_image_file"
    fi
}

# nospace_fill node
nospace_fill() {
    local node="$1"

    run "$node" dd if=/dev/zero of="$nospace_fill_file" bs=1024 count=$nospace_image_file_size || true

    return 0
}

# nospace_flush node
nospace_flush() {
    local node="$1"

    run "$node" rm -f "$nospace_fill_file"

    return 0
}
