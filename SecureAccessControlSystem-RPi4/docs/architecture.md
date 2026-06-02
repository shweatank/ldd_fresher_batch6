# Secure Access Control System - Architecture

## 1. High-Level Block Diagram

```text
+-----------------------------+        ioctl/read/write/poll       +-------------------------------+
| User-space App              |  <--------------------------------> | Char Device Driver            |
| secure_access_app           |                                      | /dev/secure_access0          |
| - Register/Login/Forgot     |                                      | - open/read/write/ioctl      |
| - SHA-256 + salt            |                                      | - wait queue + poll          |
| - SD card DB + audit log    |                                      | - sync: mutex/spinlock/atom  |
+-------------+---------------+                                      +-------+-----------------------+
              |                                                              |
              | SPI (through kernel SPI subsystem)                            | IRQ top-half
              v                                                              v
+-----------------------------+                                      +-------------------------------+
| ILI9225 TFT Controller      |                                      | ISR + Tasklet bottom-half     |
| - STATUS text messages      |                                      | - Deferred LED/TFT updates    |
+-----------------------------+                                      | - wake_up_interruptible()     |
                                                                     +-------------------------------+

+-----------------------------+
| GPIO subsystem (gpiod API)  |
| - green LED                 |
| - red LED                   |
| - wait LED                  |
+-----------------------------+

+-----------------------------+
| Device Tree + Platform drv  |
| - compatible match          |
| - SPI phandle               |
| - GPIO descriptors          |
| - IRQ mapping               |
| - MMIO resource -> ioremap  |
+-----------------------------+
```

## 2. Authentication Flow (Producer-Consumer Style)

```text
User App (producer)              Driver                    IRQ/Tasklet (consumer/deferred)
---------------------            ------                    -------------------------------
ioctl AUTH_REQUEST  ---------->  state=WAITING
write "auth_wait"  ---------->  TFT="AUTHENTICATING", wait LED ON
credentials checked in app
ioctl AUTH_RESULT   ---------->  auth_res updated; trigger irq
                                 ISR (top-half, quick)
                                 -> tasklet_schedule()
                                                              tasklet:
                                                              - blink green/red
                                                              - TFT final message
                                                              - auth_done=true
                                                              - wake_up_interruptible()
poll/read wakes up <-----------  wait_event/poll condition satisfied
```

## 3. Top Half vs Bottom Half

- **Top half (ISR)**: executes fast, non-blocking, minimal logic only (set flag + schedule tasklet).
- **Bottom half (tasklet)**: deferred context for heavier operations (LED blink delays, TFT status writes, wake queues).
- **Why**: keeps interrupt latency low and avoids illegal sleeping/long execution inside ISR.

## 4. Synchronization Map

- `mutex io_lock`: serializes SPI/TFT command sequences.
- `spinlock_t state_lock`: protects shared auth state between ioctl context and IRQ/tasklet context.
- `atomic_t open_count`, `atomic_t pending_result`: lockless counters/flags.
- `semaphore users_sem`: protects open/release count updates.
- `wait_queue_head_t auth_wq`: blocks process until auth result is published.

## 5. DTS + IOREMAP Flow

1. DTS overlay describes `secure,sac-ctrl` node with GPIOs/IRQ/reg.
2. `of_match_table` triggers `probe()`.
3. `platform_get_resource(..., IORESOURCE_MEM, 0)` fetches physical range.
4. `devm_ioremap_resource()` maps to kernel virtual address.
5. Driver uses `readl()/writel()` for educational direct register access.

## 6. Production vs Learning Comparison

- **gpiod API vs raw register**
  - Production: `gpiod_*` (portable, safe, maintained).
  - Learning: raw `readl/writel` (understand hardware register model).
- **SPI subsystem vs raw SPI registers**
  - Production: SPI core handles controller details and queueing.
  - Learning: raw register pokes explain controller internals only.
- **DTS resources vs hardcoded addresses**
  - Production: DTS decouples board wiring from driver.
  - Learning: hardcoded addresses are brittle and board-specific.
