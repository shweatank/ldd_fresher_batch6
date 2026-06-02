# Linux Sensor Monitor (Driver + User Space)

This project implements a Linux character device driver and a multi-process monitoring stack.

## Project Layout

- `include/sensor_ioctl.h` - shared ioctl API definitions
- `kernel_driver/` - kernel module (`sensor_driver.c`)
- `user_space/` - daemon, logger, dashboard, alert manager
- `deploy/` - `systemd` service and `udev` rule samples

## Requirement Coverage

### Part 1 - Linux Device Driver

- Character device interface: `open`, `read`, `write`, `unlocked_ioctl`, `poll`, `release`, `fasync`, `mmap`
- Simulated sensor data:
  - Temperature and vibration generated from random values
  - Emergency event simulation in kernel thread
  - Timer + workqueue update cycle
- Interrupt/event handling:
  - Emergency simulated via kernel thread path (interrupt-like event)
  - Wait queues (`read_wq`, `event_wq`)
  - `poll/select` through `.poll`
  - Async signals via `fasync` + `SIGIO`
- IOCTL commands:
  - Start monitoring
  - Stop monitoring
  - Set threshold
  - Enable buzzer
  - Get statistics
- Procfs/Sysfs:
  - Proc node: `/proc/sensor_monitor`
  - Sysfs attrs: `status`, `threshold`
- Synchronization:
  - `spinlock_t` for sample updates
  - `mutex` for ioctl state transitions
  - `atomic_t` for emergency state
  - `completion` for emergency acknowledgment flow

### Part 2 - User Space

- `daemon`:
  - Continuous driver reads with `epoll`
  - Handles async emergency (`SIGIO`)
  - Detects threshold violations and triggers buzzer via ioctl
- `logger`:
  - Consumes shared-memory ring buffer
  - Reads FIFO stream and writes timestamped logs
- `dashboard`:
  - Live updates via UNIX domain socket
  - Alert feed via POSIX message queue
- `alert_manager`:
  - Monitors driver statistics
  - Triggers buzzer and publishes notifications
  - Writes critical event log

### Part 3 - IPC Used (>=4)

- Shared Memory (`shm_open` + mmap ring buffer)
- POSIX Message Queue (`mq_open`, `mq_send`, `mq_receive`)
- FIFO (`mkfifo` log pipeline)
- UNIX Domain Socket (dashboard live feed)
- Signals (`SIGIO` for emergency)
- EventFD (daemon internal event signaling)

### Part 4 - Threading

- POSIX threads used for:
  - Sensor read thread
  - Alert handling thread
  - Client communication thread
  - Logger service threads
- Synchronization primitives used:
  - `pthread_mutex_t`
  - `pthread_cond_t`
  - `pthread_rwlock_t`
  - POSIX `sem_t`

### Part 5 - Advanced Features (>=3)

- `mmap` support in kernel driver
- `debugfs` interface (`/sys/kernel/debug/sensor_monitor`)
- `udev` auto node rule (`deploy/99-sensor-monitor.rules`)
- `systemd` integration (`deploy/sensor-daemon.service`)

## Build

Kernel driver:

```bash
cd kernel_driver
make
sudo insmod sensor_driver.ko
```

User space:

```bash
cd user_space
make
```

## Run

Start services in order:

```bash
cd user_space
./logger &
./daemon &
./alert_manager &
./dashboard
```

Or use the helper scripts:

```bash
cd user_space
./run_all.sh
# in another terminal when done:
./stop_all.sh
```

## Driver Controls

Examples:

- Read live data: `cat /dev/sensor_char`
- Driver proc status: `cat /proc/sensor_monitor`
- Sysfs status: `cat /sys/class/sensor_class/sensor_char/status`
- Set thresholds via sysfs:
  - `echo "75 65" | sudo tee /sys/class/sensor_class/sensor_char/threshold`

## Cleanup

```bash
sudo rmmod sensor_driver
cd kernel_driver && make clean
cd ../user_space && make clean
```
