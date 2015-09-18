#!/bin/bash -e

usage() {
	echo "$0 <disk> <URL>"
	exit 1
}

if [ "x$#" != "x2" ]; then
	usage
fi
disk="$1"
url="$2"

if [ "x$UID" != "x0" ]; then
	echo "Error: This script requires root privileges!"
	exit 1
fi

# unexpected exit hook
cleanup() {
	echo "An unknown error occured!"
	exit 1
}
trap cleanup 0

download() {
	url="$1"

	curl -k "$url" --progress
}

decompress() {
	url="$1"

	if [[ $url = *.img ]]; then
		# nothing to do, just forward
		cat
		return
	fi
	if [[ $url = *.xz ]]; then
		# unxz
		xz -d
		return
	fi
	if [[ $url = *.gz ]]; then
		gzip -d
		return
	fi
}

write() {
	disk="$1"

	dd of="$disk" bs=4M iflag=fullblock oflag=sync
}

# download, decompress and write
download "$url" | decompress "$url" | write "$disk"

# clear trap
trap - 0
