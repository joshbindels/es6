obj-m += gpio.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
arm:
	$(MAKE) ARCH=arm CROSS_COMPILE=/usr/local/xtools/arm-unknown-linux-uclibcgnueabi/bin/arm-linux- \
-C /home/student/felabs/sysdev/tinysystem/linux-2.6.34 SUBDIRS=$(PWD) modules
copy:
	cp *.ko /media/student/cda59694-7c31-446c-b16c-5fe8512910fb/
	cp setup.sh /media/student/cda59694-7c31-446c-b16c-5fe8512910fb/
clean:
	$(MAKE) -C /home/student/felabs/sysdev/tinysystem/linux-2.6.34 SUBDIRS=$(PWD) clean
