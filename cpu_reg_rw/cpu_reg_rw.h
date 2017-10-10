#ifndef __CPU_REG_RW_H__
#define __CPU_REG_RW_H__

#include <linux/ioctl.h>

typedef struct
{
	unsigned int reg_base;
	unsigned int reg_ofset;
	unsigned int reg_value;
}cpu_reg_t;

#define CPU_REG_RW_IOC_MAGIC                'y'

#define CPU_REG_CMD_READ	        _IOR(CPU_REG_RW_IOC_MAGIC, 1, cpu_reg_t)
#define CPU_REG_CMD_WRITE	        _IOW(CPU_REG_RW_IOC_MAGIC, 2, cpu_reg_t)
#define CPU_REG_CMD_SET_MASK	    _IOW(CPU_REG_RW_IOC_MAGIC, 3, cpu_reg_t)
#define CPU_REG_CMD_CLR_MASK	    _IOW(CPU_REG_RW_IOC_MAGIC, 4, cpu_reg_t)


unsigned int hsx_read_cpu_reg(unsigned int reg_base, unsigned int reg_ofset);
int hsx_write_cpu_reg(unsigned int reg_base, unsigned int reg_ofset, unsigned int reg_value);
int hsx_set_cpu_reg_mask(unsigned int reg_base, unsigned int reg_ofset, unsigned int mask);
int hsx_clr_cpu_reg_mask(unsigned int reg_base, unsigned int reg_ofset, unsigned int mask);

#endif


