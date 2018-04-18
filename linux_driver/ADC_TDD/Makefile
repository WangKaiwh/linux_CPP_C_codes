obj-m := adc_drv.o
ifneq ($(KERNELRELEASE),)
#EXTRA_CFLAGS += -Wno-error=date-time
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    override LDFLAGS=
default:
	$(Q)if [ ! -z "$(wildcard *.c)" ];then $(MAKE) -C $(KERNELDIR) M=$(PWD) modules;fi
	$(Q)if [ -z $(obj-m:.o=.ko) ]; then sudo insmod $(obj-m:.o=.ko) ;fi

clean:
	rm -rf *.o *.ko *.order *.symvers *.mod.c

endif

