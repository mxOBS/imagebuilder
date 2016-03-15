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

deleterepos() {
	buildroot="$1"
	name="$2"

	if [ -f "$buildroot/etc/apt/$name" ]; then
		rm -f "$buildroot/etc/apt/$name"
	fi
}

# usage: <buildroot> <name> <url> <suffix>
addrepo() {
	buildroot="$1"
	name="$2"
	url="$3"
	shift 3
	suffix="$*"

	repofile="$buildroot/etc/apt/$name"
	touch $repofile
	echo "deb $url $suffix" >> $repofile
	echo "deb-src $url $suffix" >> $repofile
}

# define required programs
dependencies="touch"
