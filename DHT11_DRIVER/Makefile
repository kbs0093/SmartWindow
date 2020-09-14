KERNELDIR = /lib/modules/4.14.98-v7+/build

obj-m := dht11_dev.o

PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
