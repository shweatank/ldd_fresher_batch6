savedcmd_/home/pi/prudhvi/spi/ldr_mcp3208.mod := printf '%s\n'   ldr_mcp3208.o | awk '!x[$$0]++ { print("/home/pi/prudhvi/spi/"$$0) }' > /home/pi/prudhvi/spi/ldr_mcp3208.mod
