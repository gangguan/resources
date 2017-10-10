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
#include "hi_gpio.h"
#include "cpu_reg_rw.h"


#define DUMP_DBG_PN_INFO	//输出调试通用信息
#define DUMP_DBG_PW_INFO	//输出调试警告信息
#define DUMP_DBG_PE_INFO	//输出调试错误信息
//#define DUMP_DBG_PD_INFO	//输出纯粹调试

#ifdef DUMP_DBG_PN_INFO
#define HSX_PN(msg, args...) do{printk("[gpio] NOTICE! :");printk(msg, ##args);}while(0)
#else
#define HSX_PN(msg, args...) 
#endif

#ifdef DUMP_DBG_PW_INFO
#define HSX_PW(msg, args...) do{printk("[gpio] WARNING! :");printk(msg, ##args);}while(0)
#else
#define HSX_PW(msg, args...) 
#endif

#ifdef DUMP_DBG_PE_INFO
#define HSX_PE(msg, args...) do{printk("[gpio] ERROR! :");printk(msg, ##args);}while(0)
#else
#define HSX_PE(msg, args...) 
#endif

#ifdef DUMP_DBG_PD_INFO
#define HSX_PD(msg, args...) do{printk("[gpio] DBG! :");printk(msg, ##args);}while(0)
#else
#define HSX_PD(msg, args...) 
#endif

typedef enum
{	
	ID_gk_3531_hdvr_16D1 = 0x9d,

	ID_UNKOWN = 0xFF,
}hsx_dvr_board_id;

static int board_id = ID_UNKOWN;
module_param(board_id, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(board_id, "set gpio to rest nextchip or other process");

static int get_base_reg(unsigned int grp, unsigned int pin, unsigned int *base_reg)
{
	if ((grp < 0) || (grp > 18))
		return -1;
		
	if (grp == 18)
	{
		if ((pin < 0) || (pin > 5))
			return -1;
	}
	else
	{
		if ((pin < 0) || (pin > 7))
			return -1;
	}
		
	*base_reg = 0x20150000 + 0x10000*grp;
	return 0;
}

EXPORT_SYMBOL(hsx_set_gpio_dir);
int hsx_set_gpio_dir(unsigned int grp, unsigned int pin, unsigned int dir)
{
	unsigned int base_reg;
	if (get_base_reg(grp, pin, &base_reg) != 0)
	{
		HSX_PE("grp = %d pin = %d err!\n", grp, pin);
		return -1;
	}
	
	unsigned int reg_value = hsx_read_cpu_reg(base_reg, 0x400);
	if (dir == HI_GPIO_DIR_IN)
	{
		HSX_PD("grp_%d, pin_%d, dir in\n", grp, pin);
		hsx_clr_cpu_reg_mask(base_reg, 0x400, 1 << pin);
	}
	else
	{
		HSX_PD("grp_%d, pin_%d, dir out\n", grp, pin);
		hsx_set_cpu_reg_mask(base_reg, 0x400, 1 << pin);
	}

	return 0;
}

EXPORT_SYMBOL(hsx_set_gpio_state);
int hsx_set_gpio_state(unsigned int grp, unsigned int pin, unsigned int state)
{
	unsigned int base_reg;
	if (get_base_reg(grp, pin, &base_reg) != 0)
	{
		HSX_PE("grp = %d pin = %d err!\n", grp, pin);
		return -1;
	}
	
	if (state)
	{
		HSX_PD("grp_%d, pin_%d, state 1\n", grp, pin);
		hsx_set_cpu_reg_mask(base_reg, (1 << (pin+2)), 1 << pin);
	}
	else
	{
		HSX_PD("grp_%d, pin_%d, state 0\n", grp, pin);
		hsx_clr_cpu_reg_mask(base_reg, (1 << (pin+2)), 1 << pin);
	}
	
	return 0;
}

EXPORT_SYMBOL(hsx_get_gpio_state);
int hsx_get_gpio_state(unsigned int grp, unsigned int pin)
{
	unsigned int base_reg;
	if (get_base_reg(grp, pin, &base_reg) != 0)
	{
		HSX_PE("grp = %d pin = %d err!\n", grp, pin);
		return -1;
	}
	
	unsigned int reg_value = hsx_read_cpu_reg(base_reg, (1 << (pin+2)));
	if ((reg_value & (1 << pin)) != 0)
		return 1;
		
	return 0;
}

static int gpio_open(struct inode *inode, struct file *filp)
{
   return 0;		
}

static int gpio_release(struct inode *inode, struct file *filp)
{
	return 0;	
}

//static int gpio_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
long gpio_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
		unsigned int __user *argp = (unsigned int __user *)arg;
		hsx_gpio_ctl gpio_ctl;
		
    switch (_IOC_NR(cmd))
		{			
			case _IOC_NR(GPIO_SET_PIN_DIR):
				if (copy_from_user(&gpio_ctl, argp, sizeof(gpio_ctl)))
					return -1;
				hsx_set_gpio_dir(gpio_ctl.grp, gpio_ctl.pin, gpio_ctl.value);
				break;
				
			case _IOC_NR(GPIO_SET_PIN_STATE):
				if (copy_from_user(&gpio_ctl, argp, sizeof(gpio_ctl)))
					return -1;
				hsx_set_gpio_state(gpio_ctl.grp, gpio_ctl.pin, gpio_ctl.value);
				break;
				
			case _IOC_NR(GPIO_GET_PIN_STATE):
				if (copy_from_user(&gpio_ctl, argp, sizeof(gpio_ctl)))
					return -1;
				gpio_ctl.value = hsx_get_gpio_state(gpio_ctl.grp, gpio_ctl.pin);
				copy_to_user(argp, &gpio_ctl, sizeof(gpio_ctl));
				break;
				
			default:
				printk("[gpio] cmd_0x%x not support!\n", cmd);
				break;
		}

    return 0;

}


static struct file_operations gpio_fops = {
  owner:THIS_MODULE,
  open:gpio_open,
//  ioctl:gpio_ioctl,
	unlocked_ioctl:gpio_ioctl,
  release:gpio_release,
};

static struct miscdevice gpio_dev = {
    MISC_DYNAMIC_MINOR,
    "hi_gpio",
    &gpio_fops,
};



//gpio的复用关系要放在uboot下面；
static int __init hi_gpio_init(void)
{
	signed int  ret=0;
	
	HSX_PN("board_id = 0x%x\n", board_id);
	
	switch (board_id)
	{
		case ID_gk_3531_hdvr_16D1: //GPIO_13_7控制复位
			hsx_write_cpu_reg(0x200F0000, 0x1C4, 0); //切换GPIO
			hsx_set_gpio_dir(13, 7, HI_GPIO_DIR_OUT);
			
			hsx_set_gpio_state(13, 7, 0);
			msleep(50);
			
			hsx_set_gpio_state(13, 7, 1);
			msleep(50);
			break;
			
		default:
			break;
	}

  ret = misc_register(&gpio_dev);
  if (ret)
  {
    printk(KERN_ERR "register misc dev for i2c fail!\n");
		return ret;
	}
	
	return 0;         
}


static void __exit hi_gpio_exit(void)
{
    misc_deregister(&gpio_dev);
}

module_init(hi_gpio_init);
module_exit(hi_gpio_exit);

MODULE_AUTHOR("Digital Media Team ,Hisilicon crop ");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Real Time Clock interface for HI3511");




