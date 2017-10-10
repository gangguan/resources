
ifeq ($(PARAM_FILE), )
		PARAM_FILE:=../../Makefile.param
		include $(PARAM_FILE)
endif

obj-m := cpu_reg_rw.o
cpureg_rw-y += cpu_reg_rw.o

EXTRA_CFLAGS += -D$(HI_FPGA) 

default:
	@make -C $(LINUX_ROOT) M=$(PWD) modules
clean: 
	@make -C $(LINUX_ROOT) M=$(PWD) clean