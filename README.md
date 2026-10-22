# ARC-loTDK-alcohol-Sensor
基于ARC loTDK开发板的嵌入式大作业，最终目标是用酒精浓度传感器感知酒精蒸汽的浓度，然后在ARC芯片处理后显示在OLED屏幕上
# ARC 系列嵌入式芯片开发环境搭建与库函数查询指南

本文档整理了在 Ubuntu 上搭建 ARC 系列嵌入式芯片开发环境、使用 ARC GNU 工具链、以及查询 I2C/GPIO/ADC 等外设库函数的方法。

---

## 1. 搭建 ARC GNU 工具链环境

### 1.1 下载与安装
访问 ARC GNU Toolchain 的 GitHub Releases 页面，下载适用于 Linux x86_64 的预编译包（官方支持 Ubuntu 22.04）。解压到指定目录，例如：

```bash
tar -xzf toolchain.tar.gz -C /opt/
```
1.2 配置环境变量

将工具链的 bin 目录添加到 PATH。编辑 ~/.bashrc，在末尾添加：
```bash

export PATH=/opt/arc-gnu-toolchain/bin:$PATH
```
执行 source ~/.bashrc 使配置生效。
1.3 验证路径是否生效

使用以下命令确认工具链已被系统识别：
```bash

which arc-elf32-gcc
```
如果输出完整路径（例如 /home/cyberangel/arc_gnu_2026.03_ide_linux_install/bin/arc-elf32-gcc），说明配置成功。也可以运行 arc-elf32-gcc --version 查看版本信息。

注意：如果使用 sudo 执行构建命令，sudo 默认不会继承用户 PATH，可能导致找不到编译器，建议在普通用户下编译。
1.4 安装可选依赖

若计划使用 nSIM 模拟器，需要安装 pyelftools：
```bash

sudo apt update
sudo apt install python3-pyelftools
```
2. 工具链的使用方法
2.1 基本编译命令

工具链前缀通常为 arc-elf32-。编译时需通过 -mcpu 指定目标处理器型号。例如为 ARC HS 系列开发板编译：
```bash

arc-elf32-gcc -mcpu=hs -specs=hsdk.specs main.c -o main
```
2.2 在 nSIM 模拟器上运行与调试

需要打开两个终端：

终端 1：启动 nSIM（GDB 服务器模式）
```bash

$NSIM_HOME/bin/nsimdrv -gdb -port 51000 \
  -tcf $NSIM_HOME/etc/tcf/templates/em6_gp.tcf -on nsim_emt
```
终端 2：编译并用 GDB 连接
```bash

arc-elf32-gcc -mcpu=arcem -g --specs=nsim.specs hello_world.c
arc-elf32-gdb --quiet a.out
(gdb) target remote :51000
(gdb) load
(gdb) break main
(gdb) continue
```

关键注意：

* 模拟 HS 系列核心时，nSIM 需额外添加 -on nsim_isa_ll64_option。

* nsim.specs 用于与模拟器 I/O 交互；nosys.specs 用于无 I/O 的裸机环境。

2.3 其他常用工具

将 ELF 转换为纯二进制格式以便烧录：
```bash

arc-elf32-objcopy -O binary input.elf output.bin
```
3. 支持的编程语言与基础库
3.1 编程语言

* C / C++：最核心、最通用的选择，GNU 工具链和 MetaWare 工具包均以 C/C++ 为主。

* OpenCL C / Vector C：针对具备向量处理或 NPU 能力的特定 ARC IP（如 ARC VPX DSP、ARC NPX NPU），MetaWare MX/EV 工具包提供 OpenCL C 编译器。

* 汇编：用于极致优化或启动代码，工具链中的 Binutils 支持 ARC 汇编。

3.2 基础库函数

arc-elf32-gcc 配套的是 Newlib 标准库，专为裸机嵌入式应用设计：

* 包含 printf、memcpy、strlen 等标准 C 库函数，可直接调用。

* 针对 ARC 架构和指令集进行了优化。

* 若对代码体积敏感，可通过 --specs=nano.specs 链接精简的 newlib-nano 版本。

若运行 Linux，工具链会使用 glibc 或 uClibc-ng；若使用 Synopsys MetaWare 商业工具包，则提供优化的 ANSI C/C++ 库及 STL。
4. 时钟树与外设配置要点
4.1 是否需要手动配置时钟树？

在 ARC IoTDK 上进行 C 语言开发，若无特殊时序要求，不需要手动配置时钟树。启动代码和板级支持包（BSP）会在进入 main() 之前自动完成 PLL 和分频器的初始化。只有在以下场景才需要关注时钟：

* 需要极致低功耗，动态降频。

* 完全绕过 embARC/Zephyr 启动文件，手写汇编启动代码。

* 需要修改系统时钟频率。

4.2 硬件 I2C

I2C 控制器的参考时钟固定为 16 MHz，由硬件直接提供，不经过可编程 PLL 或分频器。无需调整系统时钟来适配 I2C，驱动会基于固定 16 MHz 计算分频值，只需调用 API 设置目标波特率（如 100kHz 或 400kHz）。
4.3 硬件 ADC

ADC 挂载在 APB 总线上，时钟来源于系统总线时钟。无需重新配置 PLL，但必须通过 APBCLKEN 寄存器的 APBCLKEN[0] 位开启 ADC 时钟门控（复位后默认开启）。ADC 内部还有 ADCFDIV 分频器（位于 ADCCON 寄存器），用于将输入时钟分频至 ADC 最大工作频率 3.2 MHz，通常由驱动初始化时自动计算。
5. 如何查询 I2C、GPIO、ADC 库函数手册

目前没有独立成册的外设库函数手册，最权威的参考资料来自 embARC OSP 源码和官方文档。
5.1 在线 API 文档

访问 embARC OSP 官方文档站点：foss-for-synopsys-dwc-arc-processors.github.io/embarc_osp，查找 Device HAL 或 Driver 章节，可获取 I2C、GPIO、ADC 等外设驱动的函数原型和说明。
5.2 本地源码与头文件

在克隆的 embarc_osp 仓库中，库函数声明和注释位于 device/ 目录下。直接搜索对应头文件（如 GPIO 的 dw_gpio_obj.h）是获取函数签名和参数含义最快的方式。
5.3 示例代码

example/ 目录下提供了大量外设使用范例，位于 example/baremetal/ 或 example/freertos/ 下，查找与 i2c、gpio、adc 相关的工程文件夹，直接查看并运行示例是理解库函数调用方式最直观的方法。

注意：embARC OSP 驱动采用面向对象式设计，库函数通常通过设备对象指针（如 DEV_I2C_PTR、dw_gpio_obj）操作，找到对象定义即找到对应库函数入口。
6. 项目文件结构与新建项目
6.1 典型项目文件结构
```

    src/：存放源代码。单一应用时 main.c 和 makefile 直接放在此目录；多应用组合时每个子应用拥有独立子文件夹。

    Makefile：项目构建入口，通常基于 embARC 模板修改。

    README.md：项目说明文档。

    doc/（可选）：项目相关文档、截图或应用笔记。

    LICENSE（可选）：许可证文件。
```

6.2 新建项目方法

方法一：手动创建（基于模板）

    进入 embarc_osp/example/baremetal/ 目录。

    新建文件夹（如 hello_world）。

    将 example/example.makefile 和 example/main.c.tmpl 复制到新文件夹并重命名/修改。

    修改 makefile 中的源文件列表和项目配置。

方法二：使用 embARC CLI 自动生成（推荐）

    安装工具：pip install embarc-cli。

    创建项目：
    bash

    embarc new --directory helloworld

    工具会交互式引导选择开发板（iotdk）、版本和工具链（gnu），自动生成完整的项目目录。

创建后，在项目目录下执行 make TOOLCHAIN=gnu BOARD=iotdk 即可构建。
7. 补充：使用 embARC OSP 开发 ARC IoTDK
7.1 获取 embARC OSP
bash

git clone https://github.com/foss-for-synopsys-dwc-arc-processors/embarc_osp.git
cd embarc_osp

确保 ARC GNU 工具链的 bin 目录已加入 PATH。
7.2 编译并运行示例

以 blinky 为例：
bash

cd example/baremetal/blinky
make TOOLCHAIN=gnu BOARD=iotdk run

该命令会编译、通过 JTAG 连接开发板、下载程序并运行。IoTDK 无用户 LED，结果通过 UART 输出，可在 PuTTY 或 minicom 中查看（波特率 115200）。
7.3 调试程序
bash

make TOOLCHAIN=gnu BOARD=iotdk gui

启动基于 GDB 的图形化调试界面，可设置断点、单步执行。
7.4 底层编译选项（不依赖 embARC 时）
bash

arc-elf32-gcc -mcpu=em -specs=iotdk.specs main.c -o main

iotdk.specs 会自动配置内存布局（ICCM/DCCM 地址）和 UART 初始化代码。
7.5 调试连接与硬件设置

    调试探针：IoTDK 板载 USB 接口通过 FTDI 芯片提供 JTAG 调试和 UART 串口功能，需安装 Digilent Adept 驱动。

    串口终端：使用 PuTTY 以 115200 波特率连接对应 COM 端口观察输出。

    Zephyr 替代方案：Zephyr RTOS 也支持 IoTDK，可使用 ARC GNU 工具链编译（west build -b iotdk），调试后端默认使用 OpenOCD。
