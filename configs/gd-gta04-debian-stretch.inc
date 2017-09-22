# 
# Copyright (c) 2015-2016 Josua Mayer
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

# This is the actual cookbook

# bootstrap a debian system
bootstrap "$buildroot" stretch armhf

# add debian repos
deleterepos "$buildroot" sources.list
addrepo "$buildroot" sources.list http://httpredir.debian.org/debian stretch main non-free
addrepo "$buildroot" sources.list http://httpredir.debian.org/debian stretch-updates main non-free
addrepo "$buildroot" sources.list http://security.debian.org/ stretch/updates main non-free

# add bsp repo
#addrepo "$buildroot" sources.list.d/goldelico.list [trusted=yes] http://10.80.39.247/gta04 /

# refresh repos
chroot_run "$buildroot" apt-get update

# install HW support packages
installpkg "$buildroot" linux-image-armmp firmware-libertas

# install a bunch of useful applications
installpkg "$buildroot" sudo openssh-server nano mtd-utils psmisc ntfs-3g iw locales command-not-found
chroot_run "$buildroot" update-command-not-found

# add default user
adduser "$buildroot" debian debian /bin/bash sudo

# set hostname
sethostname "$buildroot" gta04

# set default runlevel
chroot_run "$buildroot" systemctl set-default multi-user.target

# spawn getty on serial port
chroot_run "$buildroot" systemctl enable serial-getty@ttyS0.service

# remove traces of build-system
cleanup "$buildroot"
