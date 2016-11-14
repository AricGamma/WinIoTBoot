

objects = check.o script.o update_checksum.o


edit:$(objects)
	gcc -o genchecksum_uefi $(objects) --static
	rm -rf $(objects)


clean:
	rm -rf genchecksum_uefi $(objects)
