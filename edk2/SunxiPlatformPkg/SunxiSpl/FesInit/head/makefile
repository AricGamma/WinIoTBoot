
ROOT= .
SUNXI_ROOT=$(ROOT)/../../..
SUNXI50I_ROOT=$(ROOT)/../../..
INCLUDES = -I$(SUNXI_ROOT)/Include/Interinc
INCLUDES += -I$(SUNXI50I_ROOT)/Include/boot0_include

CFLAGS   :=$(CFLAGS) $(INCLUDES)

objects = gen_fes_head.o fes_head.o 

edit:$(objects)
	gcc -o  gen_fes_head -static  $(objects) 
	./gen_fes_head
	rm -rf gen_fes_head $(objects)
clean:
	rm -rf gen_fes_head $(objects)
