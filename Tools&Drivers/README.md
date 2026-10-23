# 开发工具链

## ARC GNU编译器

Github Release页面：[ARC GNU 2026](https://github.com/foss-for-synopsys-dwc-arc-processors/toolchain/releases)

解压后需要添加`bin`目录的地址到系统的`PATH`环境变量

## embARC OSP硬件抽象层与官方源码

这个库包含了所有ARC系列芯片和开发板的库函数和宏定义，其中包含大量的示例程序

原代码地址：[embarc_osp](https://github.com/foss-for-mips-arc-processors/embarc_osp)

代码压缩包地址：[embARC OSP压缩包](https://github.com/foss-for-synopsys-dwc-arc-processors/embarc_osp/releases)

克隆到任意地址都可以正常使用，但需要将本项目放在`embarc_osp/application`中

## Digilent Adept Runtime驱动

IoTDK 板载的 FTDI 芯片通过 Digilent 的 JTAG-USB 协议与主机通信

[Digilent 官方参考页](https://digilent.com/reference/software/adept/start)

本项目使用的是x86-64的Ubuntu系统,`.deb`安装包放在本文件的同级目录

Ubuntu 需要安装 Digilent Adept 的运行时库和 udev 规则，才能让普通用户（非 root）访问该设备
