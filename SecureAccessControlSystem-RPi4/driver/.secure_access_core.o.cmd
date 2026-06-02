savedcmd_secure_access_core.o := ld -m elf_x86_64 -z noexecstack --no-warn-rwx-segments   -r -o secure_access_core.o @secure_access_core.mod 
