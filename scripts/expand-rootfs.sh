#!/bin/bash
#
#  MIT License
#  
#  Copyright (c) 2017 Josua Mayer <josua.mayer97@gmail.com>
#  
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#  
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
#  
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.
#

# For starters, find root device
rootdev="$(find_root -q)"
if [ $? -ne 0 ]; then
	# Hm. Looks like it failed, too bad.
	exit 1
fi

# now expand root partition if possible
expand_root_partition
if [ $? -ne 0 ]; then
	# Hm. Looks like it failed, too bad.
	exit 1
fi

# reload partition table
partprobe
if [ $? -ne 0 ]; then
	# Hm. Looks like it failed, too bad.
	exit 1
fi

# finally resize filesystem. Hope it is an extY
# TODO: identify root filesystem, and figure out actuial resize command
resize2fs -f "$rootdev"
if [ $? -ne 0 ]; then
	# Hm. Looks like it failed, too bad.
	exit 1
fi

# All good!
exit 0
