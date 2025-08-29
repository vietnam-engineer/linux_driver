all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

debug:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) CFLAGS_DBG="-DDEBUG" modules

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean
