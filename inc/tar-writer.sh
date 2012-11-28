#!/bin/bash

# tar_output_file file_name read_from_file file|file_content
tar_add_file() {
(
    ord() {
       printf '%u' "'$1"
    }

    strsum() {
        local strsum=0

        for ((i=0; i<${#1}; i++));do
            strsum=$(($strsum + `ord "${1:$i:1}"`))
        done

        echo $strsum
    }

    LC_CTYPE=C

    local file_name="$1"
    local read_from_file="$2"
    local file_content="$3"
    local pad
    local header
    local chsum
    local flen
    local fleno
    local now
    local oldctype
    local ctype_defined
    local accessrights
    local uid
    local gid
    local datetime

    if $read_from_file;then
        accessrights=`stat -c%a "$file_content"`
        gid=`stat -c%g "$file_content"`
        uid=`stat -c%u "$file_content"`
        flen=`stat -c%s "$file_content"`
	datetime=`stat -c%y "$file_content"`
    else
	accessrights=644
	uid=0
	gid=0
        flen=${#file_content}
        datetime="now"
    fi

    accessrights=`printf "%07u" $accessrights`
    uid=`printf "%07o" $uid`
    gid=`printf "%07o" $gid`
    fleno=`printf "%011o" $flen`
    datetime=`date --date="$datetime" '+%s'`
    datetime=`printf "%011o" $datetime`
    chsum=$((`strsum "$file_name"` + `strsum "$accessrights"` + `strsum "$uid"` + \
	`strsum "$gid"` + `strsum "$fleno"` + `strsum "$datetime"` + `strsum "        "`))

    # Output
    echo -n "$file_name"
    for ((i=0; i<$((100 - ${#file_name})); i++));do
	echo -n -e '\x00'
    done

    echo -n -e "$accessrights\x00"
    echo -n -e "$uid\x00"
    echo -n -e "$gid\x00"
    echo -n -e "$fleno\x00"
    echo -e -n "$datetime\x00"
    printf "%06o\x00 " "$chsum"

    for ((i=0; i<$((255 + 100 + 1)); i++));do
	echo -n -e '\x00'
    done

    if $read_from_file;then
        cat "$file_content"
    else
	echo -n "$file_content"
    fi

    if [ "$(($flen % 512))" != "0" ];then
        pad=$((512 - $flen % 512))
        for ((i=0; i<$pad; i++));do
	    echo -n -e '\x00'
        done
    fi
)
}

tar_close() {
    for ((i=0; i < 1024; i++));do
	echo -n -e '\x00'
    done
}
