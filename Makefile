ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m := hello.o
else
# normal makefile
KDIR ?= /lib/modules/$(shell uname -r)/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

checkpatch:
	$(KDIR)/scripts/checkpatch.pl --no-tree -f hello.c

.PHONY: default clean checkpatch
endif
