#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define GPIO_360 23
#define DEV_NAME "test_motor360_dev"

static int major;

int servo360_open(struct inode *pinode, struct file *pfile){
  printk("[SERVO360] Open servo_dev\n");
  if(gpio_request(GPIO_360, "servo360") != 0){         
    printk("[SERVO360]Already being used");
    return -1;
  }
  gpio_direction_output(GPIO_360, 0);
  return 0;
}

int servo360_close(struct inode *pinode, struct file *pfile){
  printk("[SERVO360] Close servo_dev\n");
  gpio_free(GPIO_360);  // Release the pin
  return 0;
}

void turn360_servo(int mode){     // mode  0 => turn 90 degree, mode 1 => turn -90 degree
  int i;

  if(mode == 0){        // Turn 90 degrees
    for(i = 0; i < 17; i++){
      gpio_set_value(GPIO_360, 1);
      usleep_range(1300, 1300);
      gpio_set_value(GPIO_360, 0);
      mdelay(20);
    }
  }

  else if(mode == 1){       // Turn -90 degrees
    for(i=0; i < 10; i++){
      gpio_set_value(GPIO_360, 1);
      usleep_range(1700, 1700);
      gpio_set_value(GPIO_360, 0);
      mdelay(20);
    }
  }
}

ssize_t servo360_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset){
  char get_msg;          

  if(copy_from_user(&get_msg, buffer, length) < 0){
    printk(KERN_ALERT "[SERVO360] Write error\n");
    return -1;
  }

  if(get_msg == '0'){      // Turn 90 degrees
    turn_servo(0);
    printk("[SERVO360] Turn 90 degrees\n");
  }

  else if(get_msg == '1'){    // Turn 180 degress
    turn_servo(0);
    msleep(800);
    turn_servo(0);
    printk("[SERVO360] Turn 180 degrees\n");
  }

  else if(get_msg == '2'){      // Turn -90 degrees
    turn_servo(1);
    printk("[SERVO360] Turn -90 degrees\n");
  }
  
  return 0;
}

struct file_operations fop = {
  .owner = THIS_MODULE,
  .open = servo360_open,
  .write = servo360_write,
  .release = servo360_close,
};

int __init servo360_init(void){
  major = register_chrdev(0, DEV_NAME, &fop);
  printk("Initialize SERVO360_dev major number = %d\n",major);
  return 0;
}

void __exit servo360_exit(void){
  printk("Exit SERVO360_dev\n");
  unregister_chrdev(major, DEV_NAME);
}

module_init(servo360_init);
module_exit(servo360_exit);
MODULE_LICENSE("GPL");

