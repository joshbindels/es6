obj-m += protocol.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
arm:
	$(MAKE) ARCH=arm CROSS_COMPILE=/usr/local/xtools/arm-unknown-linux-uclibcgnueabi/bin/arm-linux- -C /home/student/felabs/sysdev/tinysystem/linux-2.6.34 SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C /home/student/felabs/sysdev/tinysystem/linux-2.6.34 SUBDIRS=$(PWD) clean

