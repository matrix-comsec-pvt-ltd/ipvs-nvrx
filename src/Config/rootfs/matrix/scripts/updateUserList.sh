#!/bin/bash
ROOT_USER=root
CHROOT_LIST=/etc/vsftpd.chroot_list
if [ "$1" ];then
	echo $ROOT_USER > $CHROOT_LIST
	for user in $*
	do
		echo $user >> $CHROOT_LIST
	done
else
	echo "Please! provide your username as argument"
fi
