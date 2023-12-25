obj-m += seg_driver.o
obj-m += gpio_driver.o
RESULT1 = counter_with_button
SRC1 = $(RESULT1).c


all:
	make -C $(HOME)/working/kernel M=$(PWD) modules
	aarch64-linux-gnu-gcc -o $(RESULT1) $(SRC1)
clean:
	make -C $(HOME)/working/kernel M=$(PWD) clean
	rm -f $(RESULT1)
