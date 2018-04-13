obj-m := adc_drv.o
ifneq ($(KERNELRELEASE),)
EXTRA_CFLAGS += -Wno-error=date-time
else

    override LDFLAGS=
default:
	$(Q)if [ ! -z "$(wildcard *.c)" ];then $(MAKE) -C $(KERNELDIR) M=$(PWD) modules;fi
	$(Q)if [ -d ${MODULEPATH} ];then cp $(obj-m:.o=.ko) ${MODULEPATH}; fi

endif
include $(PRJ_PATH)/PKConfig/Lx_Script/DrvTemplate.mk
