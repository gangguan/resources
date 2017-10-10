#include <linux/module.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/fcntl.h>

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>

#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/io.h>

#include "cpu_reg_rw.h"

#define DUMP_DBG_PN_INFO	//输出调试通用信息
#define DUMP_DBG_PW_INFO	//输出调试警告信息
#define DUMP_DBG_PE_INFO	//输出调试错误信息
//#define DUMP_DBG_PD_INFO	//输出纯粹调试

#ifdef DUMP_DBG_PN_INFO
#define HSX_PN(msg, args...) do{printk("[reg_rw] NOTICE! :");printk(msg, ##args);}while(0)
#else
#define HSX_PN(msg, args...) 
#endif

#ifdef DUMP_DBG_PW_INFO
#define HSX_PW(msg, args...) do{printk("[reg_rw] WARNING! :");printk(msg, ##args);}while(0)
#else
#define HSX_PW(msg, args...) 
#endif

#ifdef DUMP_DBG_PE_INFO
#define HSX_PE(msg, args...) do{printk("[reg_rw] ERROR! :");printk(msg, ##args);}while(0)
#else
#define HSX_PE(msg, args...) 
#endif

#ifdef DUMP_DBG_PD_INFO
#define HSX_PD(msg, args...) do{printk("[reg_rw] DBG! :");printk(msg, ##args);}while(0)
#else
#define HSX_PD(msg, args...) 
#endif


#define HW_REG(reg)         *((volatile unsigned int *)(IO_ADDRESS(reg)))

EXPORT_SYMBOL(hsx_read_cpu_reg);
unsigned int hsx_read_cpu_reg(unsigned int reg_base, unsigned int reg_ofset)
{
	return HW_REG(reg_base + reg_ofset);
}

EXPORT_SYMBOL(hsx_write_cpu_reg);
int hsx_write_cpu_reg(unsigned int reg_base, unsigned int reg_ofset, unsigned int reg_value)
{
	HW_REG(reg_base + reg_ofset) = reg_value;
	
	HSX_PD("write reg_0x%x, 0x%x, read back is 0x%x\n", reg_base + reg_ofset, reg_value, HW_REG(reg_base + reg_ofset));
	return 0;
}

EXPORT_SYMBOL(hsx_set_cpu_reg_mask);
int hsx_set_cpu_reg_mask(unsigned int reg_base, unsigned int reg_ofset, unsigned int mask)
{
	unsigned int reg_value = HW_REG(reg_base + reg_ofset);
	reg_value |= mask;
	HW_REG(reg_base + reg_ofset) = reg_value;
	
	HSX_PD("set mask reg_0x%x, 0x%x, read back is 0x%x\n", reg_base + reg_ofset, reg_value, HW_REG(reg_base + reg_ofset));
	return 0;
}

EXPORT_SYMBOL(hsx_clr_cpu_reg_mask);
int hsx_clr_cpu_reg_mask(unsigned int reg_base, unsigned int reg_ofset, unsigned int mask)
{
	unsigned int reg_value = HW_REG(reg_base + reg_ofset);
	reg_value &= ~mask;
	HW_REG(reg_base + reg_ofset) = reg_value;
	
	HSX_PD("clr mask reg_0x%x, 0x%x, read back is 0x%x\n", reg_base + reg_ofset, reg_value, HW_REG(reg_base + reg_ofset));
	return 0;
}

//int cpu_reg_rw_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
long cpu_reg_rw_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    unsigned int __user *argp = (unsigned int __user *)arg;
	cpu_reg_t	cpu_reg;
	
	switch (_IOC_NR(cmd))
	{			
		case _IOC_NR(CPU_REG_CMD_READ):
			if (copy_from_user(&cpu_reg, argp, sizeof(cpu_reg_t)))
				return -1;
			cpu_reg.reg_value = hsx_read_cpu_reg(cpu_reg.reg_base, cpu_reg.reg_ofset);
			copy_to_user(argp, &cpu_reg, sizeof(cpu_reg_t));
			break;

		case _IOC_NR(CPU_REG_CMD_WRITE):
			if (copy_from_user(&cpu_reg, argp, sizeof(cpu_reg_t)))
				return -1;
			hsx_write_cpu_reg(cpu_reg.reg_base, cpu_reg.reg_ofset, cpu_reg.reg_value);
			break;
			
		case _IOC_NR(CPU_REG_CMD_SET_MASK):
			if (copy_from_user(&cpu_reg, argp, sizeof(cpu_reg_t)))
				return -1;
			hsx_set_cpu_reg_mask(cpu_reg.reg_base, cpu_reg.reg_ofset, cpu_reg.reg_value);
			break;
			
		case _IOC_NR(CPU_REG_CMD_CLR_MASK):
			if (copy_from_user(&cpu_reg, argp, sizeof(cpu_reg_t)))
				return -1;
			hsx_clr_cpu_reg_mask(cpu_reg.reg_base, cpu_reg.reg_ofset, cpu_reg.reg_value);
			break;
			
		default:
            HSX_PE("ioctl 0x%x not support!\n", _IOC_NR(cmd));
			return -1;
	}
	
	return 0;
}


static int cpu_reg_rw_open()
{
	return 0;
}

static int cpu_reg_rw_release()
{
	return 0;
}

static struct file_operations cpu_reg_rw_fops = {
	.owner      = THIS_MODULE,
//	.ioctl      = cpu_reg_rw_ioctl,
	.unlocked_ioctl  = cpu_reg_rw_ioctl,
	.open       = cpu_reg_rw_open,
	.release    = cpu_reg_rw_release,
};

static struct miscdevice cpu_reg_rw_dev = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= "cpu_reg_rw",
	.fops  		= &cpu_reg_rw_fops,
};

static int __init cpu_reg_rw_module_init(void)
{
	int ret;

	ret = misc_register(&cpu_reg_rw_dev);
	if (ret)
	{
		HSX_PE("could not register cpu_reg_rw_dev devices\n");
		return ret;
	}
	
	HSX_PN("cpu_reg_rw driver init successful!\n");
	return 0;
}

static void __exit cpu_reg_rw_module_exit(void)
{
	misc_deregister(&cpu_reg_rw_dev);	
}

module_init(cpu_reg_rw_module_init);
module_exit(cpu_reg_rw_module_exit);

#ifdef MODULE
//#include <linux/compile.h>
#endif
MODULE_LICENSE("GPL");

