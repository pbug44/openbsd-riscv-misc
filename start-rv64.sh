#!/bin/sh
#  Copyright (c) 2023 Peter J. Philipp
#  All rights reserved.
# 
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. The name of the author may not be used to endorse or promote products
#     derived from this software without specific prior written permission
# 
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
#  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

# This script is my current config for running qemu (riscv) on my raspberry pi
# The following packages are needed:
# $ pkg_info|grep -i -e riscv -e "risc-v"
# opensbi-1.2         RISC-V Supervisor Binary Interface
# u-boot-riscv64-2021.10p5 U-Boot firmware
#
# and the symlinks are as follows:
# $ ls -l rv64
# total 4
# lrwxr-xr-x  1 root  wheel   44 Jun  7 15:59 fw_jump.bin -> /usr/local/share/opensbi/generic/fw_jump.bin
# drwxr-xr-x  2 root  wheel  512 Jun  7 15:59 old
# lrwxr-xr-x  1 root  wheel   53 Jun  7 16:00 u-boot.bin -> /usr/local/share/u-boot/qemu-riscv64_smode/u-boot.bin
# 
# to do the initial install use -nographics instead of -display none, best just
# paste it without the while :

while : ; do
qemu-system-riscv64 \
        -display none \
        -smp 2,sockets=2,maxcpus=2  \
        -M virt -m 2G -bios /root/rv64/fw_jump.bin \
        -kernel /root/rv64/u-boot.bin \
        -device virtio-net-device,netdev=net \
        -netdev tap,id=net,script=/etc/qemu-ifup,downscript=/etc/qemu-ifdown \
        -device virtio-blk-device,drive=hd \
        -drive file=/home/riscv64.img,if=none,id=hd || exit 1

done

exit 0
