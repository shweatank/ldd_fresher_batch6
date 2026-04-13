savedcmd_kmalloc.mod := printf '%s\n'   kmalloc.o | awk '!x[$$0]++ { print("./"$$0) }' > kmalloc.mod
