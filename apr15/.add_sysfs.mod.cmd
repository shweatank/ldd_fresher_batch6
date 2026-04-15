savedcmd_add_sysfs.mod := printf '%s\n'   add_sysfs.o | awk '!x[$$0]++ { print("./"$$0) }' > add_sysfs.mod
