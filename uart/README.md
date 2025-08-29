# Trình điều khiển thiết bị cho bộ đếm vn29a80

## 1. Biên dịch

### 1-1. Biên dịch cho host (native compile)

Giả định rằng:
- Bạn đang sử dụng máy tính của mình để phát triển device driver chạy trên các máy tính khác có cùng kiến trúc CPU.
- Bạn đã cài đặt linux-headers (`sudo apt install linux-headers-$(uname -r)` hoặc `sudo apt install linux-headers-generic`)

Bước 1: Thiết lập một số biến môi trường cần thiết:

```bash
# [Host PC (WSL2)]
export KERNEL_SRC=/lib/modules/$(uname -r)/build

# hoặc
export KERNEL_SRC=/lib/modules/5.15.0-153-generic/build
```

Bước 2: Đứng từ thư mục gốc hoặc thư mục `uart`, hãy gõ lệnh:

```bash
# [Host PC (WSL2)]
make
```

Kết quả ta thu được các driver module (*.ko) trong thư mục `uart`. Ta có thể dùng lệnh `modinfo` để kiểm tra thông tin của các module này.

### 1-2. Biên dịch cho target (cross compile)

Giả định rằng:

- Bạn đang sử dụng PC (ví dụ WSL2) để phát triển device driver chạy trên các hệ nhúng dùng chip kiến trúc ARM (ví dụ ZCU102).
- Bạn đã cài đặt SDK (từ Yocto build) tại thư mục `~/local` của WSL2.

Bước 1: Thiết lập một số biến môi trường cần thiết:

```bash
# [Host PC (WSL2)]
source ~/local/environment-setup-aarch64-xilinx-linux
export KERNEL_SRC=$OECORE_TARGET_SYSROOT/usr/src/kernel
```

Bước 2: Đứng từ thư mục gốc hoặc thư mục `uart`, hãy gõ lệnh:

```bash
# [Host PC (WSL2)]
make
```

Kết quả ta thu được các driver module (*.ko) trong thư mục `uart`. Ta có thể dùng lệnh `modinfo` để kiểm tra thông tin của các module này.

## 2. Cài đặt và tải nạp

### 2-1. Cài đặt tạm thời và tải nạp thủ công

Bước 1: Tải các driver module cần thiết xuống board.

```bash
# [Host PC (WSL2)]
cd uart/
scp *.ko zcu102:/tmp/
```

Bước 2: Tải nạp các driver module vào trong kernel space.

```bash
# [Target board (ZCU102)]
cd /tmp
insmod uartdev_core.ko
```

Bạn có thể gặp lỗi sau:
```
insmod: ERROR: could not insert module uartdev_core.ko: Invalid module format
```

Lỗi này rất có thể do version magic của uartdev_core.ko không phù hợp với phiên bản Linux kernel đang chạy trên board. Ta có thể kiểm tra bằng cách xem kernel log (dùng lệnh `dmesg`). Nếu như đúng như vậy thì làm theo một trong 2 cách sau:
- Tìm kiếm và cài đặt lại SDK khác, phù hợp với phiên bản Linux kernel đang chạy trên board, sau đó biên dịch lại uartdev_core.ko.
- Sửa `UTS_RELEASE` trong tệp tin `$KERNEL_SRC/include/generated/utsrelease.h` thành phiên bản Linux kernel đang chạy trên board, sau đó biên dịch lại uartdev_core.ko. Tuy nhiên, cách này không được khuyến khích.

## 3. Kiểm thử

### 3-1. Kiểm thử tích hợp (IT test)

Mình đã thực hiện IT test trên ZCU102 Evaluation Kit. Việc đánh giá trên các target board khác như Raspberry Pi hay BeagleBoard cũng tương tự.

Bước 1: Tải các driver module cần thiết xuống board.

```bash
# [Host PC (WSL2)]
cd serial/
scp *.ko zcu102:/tmp/
```

Bước 2: Tải chương trình kiểm thử xuống board.

```bash
# [Host PC (WSL2)]
cd test/ && make && scp build/bin/test_counter_vn29a80 zcu102:/tmp
```

Bước 3: Chạy chương trình kiểm thử trên board.

```bash
# [Target board (ZCU102)]
/tmp/test_counter_vn29a80
```
