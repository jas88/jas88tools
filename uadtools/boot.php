<?php
header('Content-Type: text/plain');

function top() { echo "DEFAULT vesamenu.c32\nTIMEOUT 50\nALLOWOPTIONS 0\nPROMPT 0\n\nMENU TITLE  Netlab Boot Menu\n\n";}
function win() { echo "LABEL Windows\nMENU LABEL ^Windows 7 Pro\nCOM32 chain.c32\nAPPEND hd0 1\n\n"; }	
function lin() { echo "LABEL Ubuntu\nMENU LABEL Ubuntu ^Linux 16.04 LTS\nCOM32 chain.c32\nAPPEND hd0\n\n"; }
function tc() { echo "LABEL TinyCore\nMENU LABEL ^TinyCore\nSYSAPPEND 0x3ffff\ncom32 linux.c32 tc64\nappend initrd=tc.xz kmap=qwerty/uk\n\n"; }
function tcr() { echo "LABEL TinyCore\nMENU LABEL TinyCore - ^REIMAGE THIS SYSTEM\nSYSAPPEND 0x3ffff\ncom32 linux.c32 tc64\nappend initrd=tc.xz reimage localimage kmap=qwerty/uk\n\n"; }
function tcl() { echo "LABEL TinyCore\nMENU LABEL TinyCore - Local reimage\nSYSAPPEND 0x3ffff\ncom32 linux.c32 tc64\nappend initrd=tc.xz reimage localimage kmap=qwerty/uk\n\n"; }
function wincd() { "LABEL WindowsPE\nMENU LABEL Windows ^7 install disk\ncom32\nlinux.c32 wimboot\nappend initrdfile=bcd,boot.sdi,boot.wim\n\n"; }

if (isset($_POST['op'])) {
	switch($_POST['op']) {
		case 'log':
		move_uploaded_file($_FILES['log']['tmp_name'],'/lab/logs/pc/'.$_SERVER['REMOTE_ADDR']);
		break;
	}
	exit;
}

if (!isset($_GET['op'])) { $_GET['op']='menu'; }

switch($_GET['op']) {
	case 'badspeed':
	error_log('Bad Ethernet speed detected on '.$_SERVER['REMOTE_ADDR']."\n",3,'/lab/logs/speed');
	break;
	
	case 'menu':
	top();
	win();
	lin();
	tc();
	tcr();
	tcl();
	wincd();
	break;
	
	case 'script':
	$me=$_SERVER['SERVER_ADDR'];
	$ip=$_SERVER['REMOTE_ADDR'];
	$lab=(substr($ip,0,5)==='10.0.')?'hack':((substr($ip,0,8)==='10.1.5.1')?'df':'net');
	$check="dialog --pause \"Applying $lab image\" 10 40 5 || (touch /tmp/abortimage;exit 1) ||exit";
	switch($lab) {
		case 'hack':
		$name='H'.(explode('.',$ip)[3]);
		break;
		case 'df':
		$name='H'.(explode('.',$ip)[3]);
		break;
		$name='N'.(explode('.',$ip)[3]);
		break;
	}
	$labspec=<<<EOL
# Hacklab specific bits
#reged -CI /mnt/sda1/Windows/System32/config/SOFTWARE HKEY_LOCAL_MACHINE\\\\SOFTWARE /tmp/software.reg || true
#reged -CI /mnt/sda1/Windows/System32/config/SYSTEM HKEY_LOCAL_MACHINE\\\\SYSTEM /tmp/system.reg || true
#reged -CI /mnt/sda1/Users/amg/NTUSER.DAT HKEY_CURRENT_USER /tmp/user.reg || true
echo admin:hacklab | chroot /mnt/sda2 chpasswd
EOL;
	if ($lab!=='hack') { $labspec=''; }
	$script=<<<EOS
if [ -e /tmp/abortimage ]; then echo "Imaging aborted, delete /tmp/abortimage to proceed"
  exit
fi
$check

hdparm -W1 /dev/sda
echo 99 > /proc/sys/vm/dirty_ratio
echo 1 > /proc/sys/vm/dirty_background_ratio
echo 6000 > /proc/sys/vm/dirty_expire_centisecs
echo 100 > /proc/sys/vm/dirty_writeback_centisecs
echo 64 > /sys/block/sda/queue/read_ahead_kb
set -e
swapoff -a
mountpoint -q /mnt/sda1 && umount /mnt/sda1
mountpoint -q /mnt/sda2 && umount /mnt/sda2
mountpoint -q /mnt/sda3 && umount /mnt/sda3
mountpoint -q /mnt/sda4 && umount /mnt/sda4
if mountpoint -q /mnt/sda1 || mountpoint -q /mnt/sda2 || mountpoint -q /mnt/sda3 || mountpoint -q /mnt/sda4
then
        echo "Local filesystem still mounted, aborting!"
        exit
fi

if ! fgrep -q localimage /proc/cmdline
then
sfdisk -d /dev/sda|fgrep start|diff -q - /usr/local/etc/parts.txt||sfdisk -f /dev/sda <<EOP
label: dos
label-id: 0xcc1261e2
device: /dev/sda
unit: sectors

/dev/sda1 : start=        2048, size=   204800000, type=7, bootable
/dev/sda2 : start=   204802048, size=   102400000, type=83
/dev/sda3 : start=   307202048, size=    61440000, type=82
/dev/sda4 : start=   368642048, size=  1584881664, type=7
EOP
fi
mkdir -p /mnt/sda1 /mnt/sda2 /mnt/sda3 /mnt/sda4
dd if=/dev/zero of=/dev/sda2 bs=1M count=1
mkfs.ext4 -b 4096 -C 4096 -L linux -E lazy_itable_init -O bigalloc,dir_index,extent,filetype,flex_bg,sparse_super,uninit_bg,has_journal,sparse_super2,ext_attr /dev/sda2

mount -o noatime,nobarrier,delalloc,noinit_itable /dev/sda2 /mnt/sda2
ntfs-3g -o noatime,big_writes /dev/sda4 /mnt/sda4 || (mkntfs -fQ -L data -I -c 4096 /dev/sda4; ntfs-3g -o noatime,big_writes /dev/sda4 /mnt/sda4)

if ! fgrep -q localimage /proc/cmdline
then
if fgrep -q 1000 /sys/class/net/*/speed 2>/dev/null
then
udp-receiver --ttl 1 --portbase 4510 </dev/ttl| (cd /mnt/sda4 ; tar xf -)
else
curl http://$me/boot/boot.php?op=badspeed
rsync -a --progress $me::labimages/ /mnt/sda4/
fi
fi

(cd /mnt/sda2 ; pixz -d < /mnt/sda4/lin.txz | tar xf -)
mount --bind /dev /mnt/sda2/dev
mount --bind /proc /mnt/sda2/proc
mount --bind /sys /mnt/sda2/sys

pixz -d < /mnt/sda4/win.nxz | ntfsclone -rO /dev/sda1 - 2>&1|tr '\r' '\n'|cut -d. -f1|dialog --gauge "Restoring NTFS image..." 0 60
ntfsresize /dev/sda1
ntfs-3g /dev/sda1 /mnt/sda1
cp /etc/winhosts /mnt/sda1/Windows/System32/drivers/etc/hosts
echo \"ComputerName\"=\"$name\" >> /tmp/compname.reg
reged -CI /mnt/sda1/Windows/System32/config/SOFTWARE HKEY_LOCAL_MACHINE\\\\SOFTWARE /tmp/compname.reg || true

$labspec
cp /tmp/compname.exe /mnt/sda1/Windows/System32/ntname.exe
umount /mnt/sda1

sed -i -e 's/fs-uuid/label/g' -e 's/UUID=[0-9a-f]\{8\}-[0-9a-f]\{4\}-[0-9a-f]\{4\}-[0-9a-f]\{4\}-[0-9a-f]\{12\}/LABEL=linux/' -e 's/ [0-9a-f]\{8\}-[0-9a-f]\{4\}-[0-9a-f]\{4\}-[0-9a-f]\{4\}-[0-9a-f]\{12\}/ linux/gi' /mnt/sda2/boot/grub/grub.cfg
echo $name.local > /mnt/sda2/etc/hostname
tail -n +3 /etc/hosts > /mnt/sda2/etc/hosts
echo 'Acquire::http::Proxy "http://$me:3142";' > /mnt/sda2/etc/apt/apt.conf.d/01proxy
chroot /mnt/sda2 /usr/sbin/grub-install --target=i386-pc /dev/sda
sync
umount /mnt/sda2/dev
umount /mnt/sda2/proc
umount /mnt/sda2/sys
umount /mnt/sda2
umount /mnt/sda4
mkswap -L swap /dev/sda3
fgrep -q reimage /proc/cmdline && reboot
EOS;
	echo $script;
	break;
}
?>

