
ROOT= .
SUNXI_ROOT=$(ROOT)/../../..
INCLUDES = -I$(SUNXI_ROOT)/Include/Interinc

CFLAGS   :=$(CFLAGS) $(INCLUDES)

objects = gen_spare_head.o spare_head.o 

edit:$(objects)
	gcc -o gen_spare_head -static  $(objects) 
	./gen_spare_head
	rm -rf gen_spare_head $(objects)
clean:
	rm -rf gen_spare_head $(objects)
