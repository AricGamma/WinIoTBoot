all:
	make -C create_mbr
	make -C update_boot0
	make -C update_checksum_uefi
	make -C update_fes1
	make -C update_signature
	make -C update_simg
	make -C update_uefi

	cp create_mbr/update_mbr 				../pack_tools/update_mbr
	cp update_boot0/update_boot0 			../pack_tools/update_boot0
	cp update_checksum_uefi/genchecksum_uefi ../pack_tools/genchecksum_uefi
	cp update_fes1/update_fes1  			../pack_tools/update_fes1
	cp update_signature/signature 		../pack_tools/signature
	cp update_simg/simg 					../pack_toolssimg
	cp update_uefi/update_uefi 	../pack_tools/update_uefi
