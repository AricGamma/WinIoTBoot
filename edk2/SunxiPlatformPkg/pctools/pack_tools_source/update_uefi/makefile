

objects = check.o script.o update_uefi.o


edit:$(objects)
	gcc -o update_uefi $(objects) --static
	rm -rf $(objects)


clean:
	rm -rf update_uefi $(objects)
