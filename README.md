# АК-2. Лабораторна робота 4 — модуль ядра Linux (частина 1)

Модуль `hello` друкує `Hello, world!` задану кількість разів і запам'ятовує час
кожного друку у зв'язному списку. При вивантаженні модуль виводить час кожної події
в наносекундах і звільняє пам'ять.

## Файли

| Файл       | Призначення                                                  |
|------------|--------------------------------------------------------------|
| `hello.c`  | вихідний код модуля                                          |
| `Makefile` | збирання (kbuild), `make clean`, `make checkpatch`           |

## Параметр `count`

| Значення | Поведінка                                                        |
|----------|-------------------------------------------------------------------|
| не задано | за умовчанням `1`                                               |
| `0`       | попередження, модуль завантажується, нічого не друкує           |
| `1`…`4`   | друкує `Hello, world!` `count` разів                             |
| `5`…`10`  | попередження, потім друкує `count` разів                         |
| `> 10`    | повідомлення про помилку, `init` повертає `-EINVAL`, модуль не завантажується |

Параметр доступний тільки для читання: `/sys/module/hello/parameters/count`.

## Збирання (крос-компіляція під ARM)

Ядро має бути зібране як у лабораторній роботі 3 (`~/repos/linux-stable`, `multi_v7_defconfig`).

```bash
export ARCH=arm
export CROSS_COMPILE="ccache arm-linux-gnueabihf-"
export KDIR=$HOME/repos/linux-stable

make              # -> hello.ko
make checkpatch   # перевірка Linux Kernel Coding Style
modinfo hello.ko
```

## Запуск у QEMU

```bash
cd ~/repos/busybox/_install
KREL=$(make -s -C $KDIR kernelrelease)
mkdir -p proc sys dev root lib/modules/$KREL
touch lib/modules/$KREL/modules.dep     # без нього busybox modinfo не працює
cp ~/repos/lab4/hello.ko root/

cat > etc/init.d/rcS << 'EOF'
#!/bin/sh
mount -t proc proc /proc
mount -t sysfs sysfs /sys
mount -t devtmpfs devtmpfs /dev 2>/dev/null
EOF
chmod +x etc/init.d/rcS

find . | cpio -o -H newc | gzip > ../rootfs.cpio.gz
cd ..
qemu-system-arm -kernel _install/boot/zImage -initrd rootfs.cpio.gz \
    -machine virt -nographic -m 512 \
    --append "root=/dev/ram0 rw console=ttyAMA0,115200 mem=512M rdinit=/init"
```

У QEMU:

```sh
cd /root
modinfo hello.ko

insmod hello.ko                 # без параметра (count=1)
cat /sys/module/hello/parameters/count
rmmod hello

insmod hello.ko count=0         # попередження
rmmod hello

insmod hello.ko count=3         # між 1 і 10
rmmod hello

insmod hello.ko count=7         # між 5 і 10 — попередження
cat /sys/module/hello/parameters/count
rmmod hello

insmod hello.ko count=15        # більше 10 — -EINVAL, модуль не завантажено
lsmod
```

При `count > 10` модуль друкує повідомлення про помилку, а `insmod` завершується
з помилкою `invalid parameter` (`-EINVAL`).

## Перевірено

Ядро 7.3: `checkpatch.pl` — `total: 0 errors, 0 warnings`; крос-збирання під ARM
(`multi_v7_defconfig`) без помилок; усі сценарії insmod/rmmod прогнано в QEMU.
