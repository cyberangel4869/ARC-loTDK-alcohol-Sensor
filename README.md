# ARC-loTDK-alcohol-Sensor
基于ARC loTDK开发板的嵌入式大作业，最终目标是用酒精浓度传感器感知酒精蒸汽的浓度，然后在ARC芯片处理后显示在OLED屏幕上
## 硬件资源
### MQ-3酒精浓度传感器
##### 驱动方式：ADC直接读取输出电压
* 软件上只需要配置一路ADC外加一个定时器
* 查询MQ-3的传感器手册并线性拟合输出电压和酒精浓度的函数

$$
log_{10}(PPM)=\frac{log_{10}(R_s/R_0)-b}{m},b=0.25,m=-0.66
$$

### 0.96寸OLED屏幕
##### 驱动方式：硬件I2C总线
* 移植开源的U8G2驱动库，将arduino框架下的I2C库函数替换为ARC的硬件I2C函数
## 工具链搭建
### 工具链框架
本项目采用embARC OSP和ARC GNU工具链
* embARC OSP构建软件平台的框架和项目文件的框架，并负责调用其他工具链
* ARC GNU专门用于编译嵌入式代码和调试
### 工具链搭建命令
##### 1.embARC OSP工具链下载
```bash
git clone https://github.com/foss-for-synopsys-dwc-arc-processors/embarc_osp.git
cd embarc_osp
```
##### 2.ARC GNU工具链下载
* 从github的release界面下载linux系统的压缩包
* 找到工具链的bin目录，加入系统的PATH环境变量
* 通过`arc-elf32-gcc --version`指令检查环境变量是否配置成功，若返回ARC工具链版本则配置成功
##### 3.Digilent Adept Runtime驱动程序
IoTDK 板载的 FTDI 芯片通过 Digilent 的 JTAG-USB 协议与主机通信，Ubuntu 需要安装 Digilent Adept 的运行时库和 udev 规则，才能让普通用户（非 root）访问该设备。如果缺少它，`make run` 或 `make gui` 在尝试连接板子时会失败。安装后通常需要重新加载 udev 规则或重启系统。

## 项目说明
### 文件结构
embARC OSP工具链的库函数和板级宏定义分散在整个工具根目录下的各个文件夹中，为了保证该项目的Cmake文件可以正确的识别并编译库函数头文件，必须保证文件结构的完整性,文件结构如下
```
./embarc_osp/
├── application
│   └── alcoholSensor
├── apply_embARC_patch.sh
├── arc
│   ├── arc_cache.c
│   ├── arc_connect.c
│   ├── arc_exc_asm.s
│   ├── arc_exception.c
│   ├── arc.mk
│   ├── arc_mp.c
│   ├── arc_mpu.c
│   ├── arc_timer.c
│   ├── arc_udma.c
│   ├── arc_util.s
│   └── startup
├── board
│   ├── axs
│   ├── board.c
│   ├── board.h
│   ├── board.mk
│   ├── emsdp
│   ├── emsk
│   ├── hsdk
│   ├── iotdk
│   └── nsim
├── build
│   ├── CMakeCache.txt
│   ├── CMakeFiles
│   ├── cmake_install.cmake
│   ├── compile_commands.json
│   ├── CPackConfig.cmake
│   ├── CPackSourceConfig.cmake
│   └── Makefile
├── device
│   ├── device.mk
│   ├── ip
│   └── peripheral
├── doc
│   ├── documents
│   ├── embARC_Document.html
│   ├── embARC.doxyfile
│   └── Makefile
├── example
│   ├── baremetal
│   ├── example.makefile
│   ├── example.rst
│   ├── examples.yaml
│   ├── freertos
│   ├── main.c.tmpl
│   └── makefile
├── include
│   ├── arc
│   ├── code_gen_macros.h
│   ├── device
│   ├── embARC_assert.h
│   ├── embARC_debug.h
│   ├── embARC_error.h
│   ├── embARC.h
│   ├── embARC_test.h
│   └── embARC_toolchain.h
├── library
│   ├── clib
│   ├── library.mk
│   ├── mcuboot
│   └── secureshield
├── LICENSE
├── middleware
│   ├── aws
│   ├── coap
│   ├── common
│   ├── fatfs
│   ├── http_parser
│   ├── ihex
│   ├── lwip
│   ├── lwip-contrib
│   ├── mbedtls
│   ├── middleware.mk
│   ├── mqtt
│   ├── ntshell
│   ├── openthread
│   ├── parson
│   ├── u8glib
│   └── wakaama
├── options
│   ├── debug.mk
│   ├── files.mk
│   ├── gmsl
│   ├── options.mk
│   ├── rules.mk
│   ├── scripts.mk
│   ├── toolchain
│   └── toolchain.mk
├── os
│   ├── freertos
│   ├── freertos.mk
│   ├── hal
│   └── os.mk
├── README.md
├── scripts
│   ├── patch
│   ├── requirements_doc.txt
│   ├── requirements.txt
│   └── test.py
└── Synopsys_FOSS_Notices_embARC.pdf
```
本仓库的所有文件都应该放在`embarc_osp/application`中
