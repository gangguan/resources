
ifeq ($(PARAM_FILE), )
		PARAM_FILE:=../../Makefile.param
		include $(PARAM_FILE)
endif

obj-m := hi_gpio.o
gpio-y += hi_gpio.o

EXTRA_CFLAGS += -D$(HI_FPGA) 
EXTRA_CFLAGS += -I$(PWD)/../cpu_reg_rw

default:
	@make -C $(LINUX_ROOT) M=$(PWD) modules
clean: 
	@make -C $(LINUX_ROOT) M=$(PWD) clean