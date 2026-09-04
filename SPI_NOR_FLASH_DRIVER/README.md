**W25Q128 SPI NOR Flash Driver**  
A Linux SPI NOR flash driver project for the **Winbond W25Q128**, connected to an  **NXP i.MX8M Nano EVK**. The project provides flash read, write, and erase operations through the Linux  **Memory Technology Device (MTD)** subsystem and includes a user-space test application.  
**Project files**  
| | |  
|-|-|  
| **File** | **Purpose** |   
| w25q128.c | SPI NOR flash kernel driver and MTD integration |   
| test.c | User-space flash test application |   
| Makefile | Project build configuration |   
   
Generated files such as .ko, .o, .mod.c, Module.symvers, and the test executable are build outputs and are not required in the source repository.  
**Hardware and software**  
| | |  
|-|-|  
| **Item** | **Project configuration** |   
| Development board | NXP i.MX8M Nano EVK |   
| Processor architecture | ARM64 / AArch64 |   
| Target Linux kernel | 6.1.22 |   
| External flash | Winbond W25Q128 |   
| Flash capacity | 16 MiB (128 Mbit) |   
| Program page size | 256 bytes |   
| Erase sector size | 4 KiB |   
| Interface | SPI using ECSPI2 |   
| Cross-compiler prefix | aarch64-linux-gnu- |   
   
**Features**  
- SPI device detection through the driver's probe callback.  
- JEDEC identification read.  
- MTD registration to expose flash operations to user space.  
- Flash data reads, page programming, and sector erasure.  
- Write-enable commands and status-register polling for flash operations.  
- A private device structure containing the SPI device, MTD information, and mutex.  
- User-space testing of flash operations.  
**Hardware connections**  
The project uses the following EVK 40-pin header connections. Confirm the header orientation and board pin configuration before wiring.  
   
| | | |  
|-|-|-|  
| **EVK header pin** | **Signal** | **Flash connection** |   
| 1 | 3.3 V | VCC |   
| 6 | Ground | GND |   
| 19 | ECSPI2 MOSI | DI / MOSI |   
| 21 | ECSPI2 MISO | DO / MISO |   
| 23 | ECSPI2 SCLK | CLK |   
| 24 | ECSPI2 SS0 | CS# |   
   
For standard SPI operation, WP# and HOLD#/RESET# must be held at their appropriate inactive levels according to the exact flash package or module wiring.  
**Driver flow**  
1. The device tree enables the SPI controller and describes the attached flash.  
2. The Linux SPI subsystem matches the device to the driver and calls probe().  
3. The driver initializes its per-device state and reads the flash identification.  
4. The driver configures flash geometry and registers the MTD device.  
5. User-space operations reach the driver's callbacks through MTD.  
6. The callbacks issue SPI commands to read, program, or erase flash.  
7. Driver removal unregisters the MTD device and releases driver resources.  
**SPI commands**  
| | |  
|-|-|  
| **Operation** | **Command** |   
| Read data | 0x03 |   
| Read JEDEC ID | 0x9F |   
| Write enable | 0x06 |   
| Read status register | 0x05 |   
| Page program | 0x02 |   
| Sector erase (4 KiB) | 0x20 |   
   
Erase prepares a sector for programming. Programming changes bits from 1 to 0; erasure restores bits to 1. Program operations must respect page boundaries, and erase requests must respect sector alignment.  
**Build on the host**  
Prerequisites:  
- An AArch64 cross-compilation toolchain.  
- The configured kernel build tree matching the target board.  
- Matching kernel configuration and build symbols, including Module.symvers when symbol versioning is enabled.  
From the project directory, update the kernel build path in Makefile to match your host setup, then run:  
make  
   
The kernel module output is w25q128.ko. The application output name is determined by the Makefile.  
The application can also be compiled separately:  
aarch64-linux-gnu-gcc -Wall -Wextra test.c -o test  
   
**Device tree and driver binding**  
Enable ECSPI2 and configure its pin multiplexing, chip select, and flash child node in the board's device tree. The flash node's compatible string must match the driver's OF match table.  
In the project setup, ECSPI2 was enumerated as spi1, with the external flash at spi1.0. Bus numbering depends on the device-tree aliases and board configuration.  
Only one driver can bind to a given SPI device at a time. If the standard spi-nor driver already owns the external flash, arrange the intended binding before loading this custom driver. Loading the module alone does not replace an existing binding.  
The device-tree changes are board setup requirements; they are not included among the three source files listed above.  
**Load and inspect on the board**  
Transfer w25q128.ko and the ARM64 test executable to the board using your usual transfer method. Run the following from that directory as root:  
uname -r  
 insmod ./w25q128.ko  
 dmesg | tail -n 40  
 cat /proc/mtd  
 ls -l /dev/mtd*  
   
Check the kernel log for successful probe and MTD registration. Identify the external W25Q128 by its device association, name, and expected capacity of 16 MiB (01000000 bytes in /proc/mtd).  
**Do not assume the external flash is ** **/dev/mtd0** **.** The board may also expose an onboard flash through MTD. Never run erase or write tests on a device containing boot firmware or other required data.  
**Test procedure**  
Review test.c for its device path, command-line arguments, and test region before running it. Update any hard-coded MTD path to the verified external flash device. The invocation depends on the application's actual argument handling.  
Suggested validation sequence:  
1. Confirm successful device detection and MTD registration.  
2. Select a disposable, sector-aligned region on the external flash.  
3. Erase the selected region.  
4. Write a known data pattern.  
5. Read the data back and compare it with the original pattern.  
6. Reboot or power-cycle the board, then read the same region again without erasing or rewriting it to check persistence.  
These steps describe the validation procedure; preserve actual console output separately as evidence of test results.  
**Unload**  
Close applications using the device, then run as root:  
rmmod w25q128  
 dmesg | tail -n 20  
   
**Troubleshooting**  
| | |  
|-|-|  
| **Symptom** | **Checks** |   
| Invalid module format | Check target kernel release, architecture, configuration, and symbol versions against the module build. |   
| Module loads but probe does not run | Check device-tree matching, controller status, and whether another driver owns the device. |   
| JEDEC identification reads as all FF or 00 | Check power, ground, chip select, wiring, pin multiplexing, and SPI settings. |   
| No new MTD device | Inspect the kernel log for probe or MTD registration errors. |   
| Write or erase fails | Check device selection, bounds, alignment, write protection, and driver error messages. |   
| Read-back mismatch | Check erase-before-write handling, page boundaries, addressing, and operation completion polling. |   
   
**Learning outcomes**  
This project demonstrates Linux SPI driver development, device-tree-based device matching, hardware/software integration, MTD registration, flash command sequencing, cross-compilation, and user-space validation on an ARM64 board.  
