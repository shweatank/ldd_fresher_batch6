savedcmd_/home/pi/Satheesh/DTS/my_led_driver.mod := printf '%s\n'   my_led_driver.o | awk '!x[$$0]++ { print("/home/pi/Satheesh/DTS/"$$0) }' > /home/pi/Satheesh/DTS/my_led_driver.mod
