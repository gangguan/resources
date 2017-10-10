#ifndef __HI_GPIO_H__ 
#define __HI_GPIO_H__

#define HI_GPIO_DIR_IN 0
#define HI_GPIO_DIR_OUT 1

typedef struct
{
	unsigned int grp;
	unsigned int pin;
	unsigned int value;
}hsx_gpio_ctl;

#define GPIO_IOC_MAGIC                'h'

#define GPIO_SET_PIN_DIR	        _IOW(GPIO_IOC_MAGIC, 1, hsx_gpio_ctl)
#define GPIO_SET_PIN_STATE	      _IOW(GPIO_IOC_MAGIC, 2, hsx_gpio_ctl)
#define GPIO_GET_PIN_STATE	      _IOW(GPIO_IOC_MAGIC, 3, hsx_gpio_ctl)


int hsx_set_gpio_dir(unsigned int grp, unsigned int pin, unsigned int dir);
int hsx_set_gpio_state(unsigned int grp, unsigned int pin, unsigned int state);
int hsx_get_gpio_state(unsigned int grp, unsigned int pin);

#endif
