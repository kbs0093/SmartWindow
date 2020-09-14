#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/uaccess.h>


#define GPIO24 24
#define GPIO17 17
 
#define DEVNAME "dht11_dev"
#define DEVNUM 239
#define CHECKSUM 40 // for Checksum

static int data[5]; // for save the DHT 11 data
static bool status_err;
static int major;

static int dht11_read(int gpio_num){
    int time = 0;
    int cur_data = 0;
    int i;
    
    while(time < 150){                       // if 150us is passed then something is wrong.
        if(gpio_get_value(gpio_num) == 1)            // 'pulled ready to output' signal
          break;
        udelay(1);
        time++;
    }

    if(time == 150){                        // failed to read
        status_err = true;
        return -1;
    }

    time = 0;
    udelay(100);                        // pulled ready output signal = 80us, response signal = 50us.

    for (i = 0; i < CHECKSUM; i++){    //start signal detect!
        time = 0;

        while(time < 160){    //time over 160 (zero state 80us + one state 80us)            
          if(gpio_get_value(gpio_num) == 1)
              break;        
          udelay(1);
          time++;
        }

        time = 0; // reset the time

        while(gpio_get_value(gpio_num) == 1){   //calculate value 1 length 
          udelay(1);
          time++;

          if(time == 150){                     
            status_err = true;
            return -1;
          }
        }

        /* i/8 mean : 
        0~8 : data[0]
        9~16 : data[1]
        17~24 : data[2]
        25~32 : data[3]
        33~40 : data[4] checksum : if read is right -> checksum = data[0] + data[1] + data[2] + data[3]
        */
        cur_data = i/8; 
        data[cur_data] = data[cur_data] << 1;  // basically dht11 always output 1 bit 
                                               // so driver always *2 data value
        
        if(time > 25){// over 28us means data = 1        
            data[cur_data] = data[cur_data] + 1;
        }
    }

    return i;
}

int dht_open(struct inode *pinode, struct file *pfile){
    printk("DHT11 Driver Open Success!\n");

    if(gpio_request(GPIO17, DEVNAME)!=0){
        printk("DHT11 GPIO17 request Fail!\n");
        return -1;
    }
    if(gpio_request(GPIO24, DEVNAME)!=0){
        printk("DHT11 GPIO24 request Fail!\n");
        return -1;
    } 

  return 0;
}

int dht_close(struct inode *pinode, struct file *pfile){
	  printk("DHT11 Driver Close!\n");
	  gpio_free(GPIO17);
      gpio_free(GPIO24);

	  return 0;
}

void read_data(int gpio_num){

    status_err = false;
    // Initialize data
    data[0] = 0;  
    data[1] = 0;  
    data[2] = 0;  
    data[3] = 0;  
    data[4] = 0;  

    /*DHT11 Datasheet
        Intialize Input state
        1. set 0 (low) during 18ms
        2. set 1 (high) during 20~40us
        3. gpio pin set input mode
    */
    gpio_direction_output(gpio_num, 0);
    gpio_set_value(gpio_num, 0);
    mdelay(18);
    gpio_set_value(gpio_num, 1);
    udelay(30);
    gpio_direction_input(gpio_num);

    dht11_read(gpio_num);    
  
    if(data[4] == ((data[0] + data[1] + data[2] + data[3]) % 256)){// parity check if data0,1,2,3 sum over 2^8bit threadhold data[4] save the mod value
        if(data[0] <= 0){
			status_err = true;
		}
		
		return;
	}                                                             
    else{
        status_err = true;
        return;
	}
}

int dht_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset){
    char buf[20];
    int temp[4];
   

start:
    read_data(GPIO17);
 
    if(status_err){
        printk(KERN_INFO "DHT11_In 17 Reading failed\n");
        goto start;
    }
    
    printk("DHT11_in: Tem = %d Hum = %d\n", data[0],data[2]);
    temp [0] = data [0];
    temp [1] = data [2];
    
    
start2:
    read_data(GPIO24);
 
    if(status_err){
        printk(KERN_INFO "DHT11_Out 24 Reading failed\n");
        goto start2;
    }
    
    printk("DHT11_out: Tem = %d Hum = %d\n", data[0],data[2]);    
    temp[2] = data[0];
    temp[3] = data[2];   
      
    sprintf(buf, "%d %d %d %d\n", temp[0], temp[1],temp[2],temp[3]);    

    copy_to_user(buffer, buf, sizeof(buf));
    return 0;
}

struct file_operations fop = {
    .open = dht_open,
    .read = dht_read,
    .release = dht_close,
};

int __init dht11_init(void){
    major = register_chrdev(0, DEVNAME, &fop);
    
    if(major < 0){
        printk(KERN_INFO "DHT11_In Driver Failed!\n");
        return -1;
    }
    
    //gpio_request(GPIO, DEVNAME);    
    printk(KERN_INFO "DHT11 Driver Init Success Major Number = %d\n",major);
    return 0;
}

void __exit dht11_exit(void){
    unregister_chrdev(DEVNUM, DEVNAME);
    gpio_free(GPIO17);
    gpio_free(GPIO24);

    printk(KERN_INFO "DHT11 Driver Exit!\n");

    return;
}

module_init(dht11_init);
module_exit(dht11_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ko BeomSeok");
MODULE_DESCRIPTION("DHT11 Sensor Driver (GPIO)");
