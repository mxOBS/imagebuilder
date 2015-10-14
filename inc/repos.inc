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

# This function adds all custom repositories to a system
# usage: <buildroot> <distro-codename>
add_repos() {
	buildroot=$1
	distro=$2

	# start with distribution default repos
	repofile=$buildroot/etc/apt/sources.list
	if [ "$distro" = "wheezy" ]; then
		echo "deb http://httpredir.debian.org/debian wheezy main" > $repofile
		echo "deb-src http://httpredir.debian.org/debian wheezy main" >> $repofile
		echo "deb http://httpredir.debian.org/debian wheezy-updates main" >> $repofile
		echo "deb-src http://httpredir.debian.org/debian wheezy-updates main" >> $repofile
		echo "deb http://security.debian.org/ wheezy/updates main" >> $repofile
		echo "deb-src http://security.debian.org/ wheezy/updates main" >> $repofile
	fi
	if [ "$distro" = "jessie" ]; then
		echo "deb http://httpredir.debian.org/debian jessie main" > $repofile
		echo "deb-src http://httpredir.debian.org/debian jessie main" >> $repofile
		echo "deb http://httpredir.debian.org/debian jessie-updates main" >> $repofile
		echo "deb-src http://httpredir.debian.org/debian jessie-updates main" >> $repofile
		echo "deb http://security.debian.org/ jessie/updates main" >> $repofile
		echo "deb-src http://security.debian.org/ jessie/updates main" >> $repofile
	fi
	if [ "$distro" = "trusty" ]; then
		echo "deb http://ports.ubuntu.com/ubuntu-ports/ trusty main" > $repofile
		echo "deb-src http://ports.ubuntu.com/ubuntu-ports/ trusty main" >> $repofile
		echo "deb http://ports.ubuntu.com/ubuntu-ports/ trusty-updates main" >> $repofile
		echo "deb-src http://ports.ubuntu.com/ubuntu-ports/ trusty-updates main" >> $repofile
		echo "deb http://ports.ubuntu.com/ubuntu-ports/ trusty-security main" >> $repofile
		echo "deb-src http://ports.ubuntu.com/ubuntu-ports/ trusty-security main" >> $repofile
	fi

	# i.MX6 BSP repo
	repofile=$buildroot/etc/apt/sources.list.d/imx6.list
	prefsfile=$buildroot/etc/apt/preferences.d/imx6
	if [ "$distro" = "wheezy" ]; then
		echo "deb https://repo.solid-build.xyz/debian/wheezy/bsp-imx6/ ./" > $repofile
		echo "deb-src https://repo.solid-build.xyz/debian/wheezy/bsp-imx6/ ./" >> $repofile
		echo "Package: *" > $prefsfile
		echo "Pin: release o=obs://mx6bs/BSP:Cubox-i/Debian_Wheezy" >> $prefsfile
		echo "Pin-Priority: 600" >> $prefsfile
		curl -L https://repo.solid-build.xyz/debian/wheezy/bsp-imx6//Release.key | chroot_run $buildroot "apt-key add -"
	fi
	if [ "$distro" = "jessie" ]; then
		echo "deb https://repo.solid-build.xyz/debian/jessie/bsp-imx6/ ./" > $repofile
		echo "deb-src https://repo.solid-build.xyz/debian/jessie/bsp-imx6/ ./" >> $repofile
		echo "Package: *" >> $prefsfile
		echo "Pin: release o=obs://mx6bs/BSP:Cubox-i/Debian_Jessie" >> $prefsfile
		echo "Pin-Priority: 600" >> $prefsfile
		curl -L https://repo.solid-build.xyz/debian/jessie/bsp-imx6//Release.key | chroot_run $buildroot "apt-key add -"
	fi
	if [ "$distro" = "trusty" ]; then
		echo "deb https://repo.solid-build.xyz/obs/BSP:/IMX6/Ubuntu_Trusty/ ./" > $repofile
		echo "deb-src https://repo.solid-build.xyz/obs/BSP:/IMX6/Ubuntu_Trusty/ ./" >> $repofile
		echo "Package: *" >> $prefsfile
		echo "Pin: release o=obs://mx6bs/BSP:Cubox-i/Ubuntu_Trusty" >> $prefsfile
		echo "Pin-Priority: 600" >> $prefsfile
		curl -L https://repo.solid-build.xyz/obs/BSP:/IMX6/Ubuntu_Trusty/Release.key | chroot_run $buildroot "apt-key add -"
	fi

	# refresh apt cache
	chroot_run $buildroot "apt-get update"
}
