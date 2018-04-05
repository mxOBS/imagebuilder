# 
# Copyright (c) 2015 Josua Mayer
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

# usage: <buildroot> <packages>
installpkg() {
	buildroot="$1"
	shift 1
	packages="$*"

	# block apt from running system services
	echo "exit 101" > "$buildroot/usr/sbin/policy-rc.d"
	chmod 777 "$buildroot/usr/sbin/policy-rc.d"

	# actually install packages
	chroot_run "$buildroot" apt-get -y install $packages

	# unblock system services
	rm -f "$buildroot/usr/sbin/policy-rc.d"
}

# usage: <buildroot> <debs>
installpkg_local() {
	buildroot="$1"
	shift 1
	debs="$*"

	# block apt from running system services
	echo "exit 101" > "$buildroot/usr/sbin/policy-rc.d"
	chmod 777 "$buildroot/usr/sbin/policy-rc.d"

	# copy debs to target system
	cp -L $debs "$buildroot/"

	# actually install debs
	chroot_run "$buildroot" dpkg -i $debs

	# delete debs
	for deb in $debs; do
		rm -f "$buildroot/$deb"
	done

	# unblock system services
	rm -f "$buildroot/usr/sbin/policy-rc.d"
}

# usage: <buildroot> <urls>
installpkg_url() {
	buildroot="$1"
	shift 1

	# loop through arguments
	while [ 1 -le $# ]; do
		url="$1"

		# fetch deb
		wget -O "$buildroot/installpkg_url.deb" "$url"

		# block apt from running system services
		echo "exit 101" > "$buildroot/usr/sbin/policy-rc.d"
		chmod 777 "$buildroot/usr/sbin/policy-rc.d"

		# actually install deb
		chroot_run "$buildroot" dpkg -i installpkg_url.deb

		# unblock system services
		rm -f "$buildroot/usr/sbin/policy-rc.d"

		# delete deb
		rm "$buildroot/installpkg_url.deb"

		# next
		shift 1
	done
}

# define required programs
dependencies="rm chmod wget"
