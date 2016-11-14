ROOT= .
SUNXI_ROOT=$(ROOT)/../../..
SUNXI50I_ROOT=$(ROOT)/../../..
INCLUDES = -I$(SUNXI_ROOT)/Include/Interinc
INCLUDES += -I$(SUNXI50I_ROOT)/Include/boot0_include

CFLAGS   :=$(CFLAGS) $(INCLUDES)

objects = gen_boot0_head.o boot0_head.o 

edit:$(objects)
	gcc -o gen_boot0_head -static  $(objects) 
	./gen_boot0_head
	rm -rf gen_boot0_head $(objects)
clean:
	rm -rf gen_boot0_head $(objects)
