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
bootstrap "$buildroot" testing armhf

# add debian repos
deleterepos "$buildroot" sources.list
addrepo "$buildroot" sources.list http://httpredir.debian.org/debian testing main non-free
addrepo "$buildroot" sources.list http://httpredir.debian.org/debian testing-updates main non-free
addrepo "$buildroot" sources.list http://security.debian.org/ testing/updates main non-free

# refresh repos
chroot_run "$buildroot" apt-get update

# debconf preseed
debconf_set_selection "$buildroot" locales locales/default_environment_locale select en_US.UTF-8
debconf_set_selection "$buildroot" locales locales/locales_to_be_generated multiselect en_US.UTF-8 UTF-8
debconf_set_selection "$buildroot" localepurge localepurge/nopurge multiselect en, en_US.UTF-8
debconf_set_selection "$buildroot" localepurge localepurge/use-dpkg-feature boolean true
debconf_set_selection "$buildroot" localepurge localepurge/mandelete boolean true

# disable installing additional packages
cat > "$buildroot/etc/apt/apt.conf.d/99tiny" << EOF
APT::Install-Recommends "0";
APT::Install-Suggests "0";
EOF

# install HW support packages
installpkg "$buildroot" linux-image-armmp firmware-libertas firmware-ti-connectivity mtd-utils

# install a bunch of useful applications
installpkg "$buildroot" sudo openssh-server mtd-utils rfkill psmisc nano iw locales localepurge
# TODO: how to trigger first run of localepurge if USE_DPKG was set? saves 30MB
chroot_run "$buildroot" localepurge

# add default user
adduser "$buildroot" debian debian /bin/bash sudo

# set hostname
sethostname "$buildroot" gta04

# set default runlevel
chroot_run "$buildroot" systemctl set-default multi-user.target

# spawn getty on serial port
chroot_run "$buildroot" systemctl enable serial-getty@ttyO2.service

# remove traces of build-system
cleanup "$buildroot"
