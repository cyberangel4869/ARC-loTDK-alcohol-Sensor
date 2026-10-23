# 搭建开发工具链

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

本项目使用的是x86-64的Ubuntu系统，安装包放在`./digilent.adept.runtime_2.30.1_amd64.deb`,安装命令

```bash
sudo dpkg -i ./digilent.adept.runtime_2.30.1_amd64.deb
```

为了可以用终端检查开发板上的JTAG芯片与PC的通讯是否正常，还需要安装Utilities软件，在上述官方网站的utilities分区可以下载安装包

本项目使用的安装包放在`./digilent.adept.utilities_2.7.1-amd64.deb`，安装命令

```bash
sudo dpkg -i ./digilent.adept.utilities_2.7.1-amd64.deb
sudo udevadm control --reload-rules
```
用USB数据线链接IoTDK开发板和PC，运行以下命令

```bash
djtgcfg enum
```
如果识别到了设备，则驱动安装成功

**NOTE：** 较新版本的Ubuntu系统与预编译的GDB包之间可能存在兼容性问题，解决方案如下：
* 尝试运行`sudo apt install libncursesw5`来安装旧版的库
* 若旧版库已经被移除，则需要通过软链接来把新版库接入GDB，运行下面的命令
  ```bash
  sudo ln -s /usr/lib/x86_64-linux-gnu/libncursesw.so.6 /usr/lib/x86_64-linux-gnu/libncursesw.so.5
  sudo ln -s /usr/lib/x86_64-linux-gnu/libtinfo.so.6 /usr/lib/x86_64-linux-gnu/libtinfo.so.5
  ```
  然后用`arc-elf32-gdb --version`验证GDB能否启动，若返回版本信息，则兼容性问题已解决
