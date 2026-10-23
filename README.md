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
* Digilent Adept Runtime用于驱动IoTDK板载的JTAG芯片，实现程序烧录和调试的功能
### 工具链搭建
[工具链搭建指南](./Tools&Drivers/README.md)
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

### OpenOCD相关配置文件修改
OpenOCD在程序烧录中承担与板载芯片建立JTAG协议链接，响应GDB的load指令，并将ELF分段写入ARC芯片指定地址的关键作用

较新版本的ARC GNU工具的文件结构发生改变，embARC OSP的配置文件中却还在用旧路径调用OpenOCD，这会使程序下载因无法调用OpenOCD组件而失败

相关配置代码位于`embarc_osp/options/debug.mk`配置文件中，代码如下

```makefile

ifeq ($(VALID_JTAG), usb)
DBG_HW_FLAGS += -ex "target remote | openocd --pipe $(OPENOCD_OPTIONS)" -ex "load"
endif
```
问题根源在于 debug.mk 引用了一个 `OPENOCD_OPTIONS` 变量，该变量定义在 toolchain_gnu.mk 中，依然沿用旧的路径，路径多出了 `bin/`

而`debug.mk`中的路径变量`DBG_HW_FLAGS`被用于合成调用OpenOCD的指令，它的错误会让所有相关指令都因找不到文件而报错，进而使得烧录程序崩溃退出

为了修复这一问题，需要手动传入OpenOCD的根目录。同时为了避免路径合成出错，直接指定为绝对路径，具体做法如下：
* 首先需要确认OpenOCD相关文件的位置
  * 先进入解压ARC GNU工具的目录:`cd ~/arc_gnu_2026.03_ide_linux_install/`，可用Tab键自动补全路径
  * 然后寻找OpenOCD的位置：`find ./ -name "em.tcl"`
    ```bash
    cyberangel@cyberangel-B550M-GAMING-ARGB:~/arc_gnu_2026.03_ide_linux_install$ find ./ -name "em.tcl"
    ./share/openocd/scripts/cpu/arc/em.tcl
    ```
  * `scripts`目录即为OpenOCD的根目录，最后需要用`pwd`指令获取根目录的绝对地址：
    ```bash
    cyberangel@cyberangel-B550M-GAMING-ARGB:~/arc_gnu_2026.03_ide_linux_install$ cd ./share/openocd/scripts/
    cyberangel@cyberangel-B550M-GAMING-ARGB:~/arc_gnu_2026.03_ide_linux_install/share/openocd/scripts$ pwd
    /home/cyberangel/arc_gnu_2026.03_ide_linux_install/share/openocd/scripts
    ```
* 然后就可以直接把绝对地址作为参数传递给`OPENOCD_OPTIONS`变量，从而保证后续的指令合成正确

```makefile
ifeq ($(VALID_JTAG), usb)
# 工具链安装目录下的 openocd scripts 实际在 share/ 下，而非 bin/share/，此处直接指定绝对路径
#DBG_HW_FLAGS += -ex "target remote | openocd --pipe $(OPENOCD_OPTIONS)" -ex "load"
DBG_HW_FLAGS += -ex "target remote | openocd --pipe -s /home/cyberangel/arc_gnu_2026.03_ide_linux_install/share/openocd/scripts -f $(OPENOCD_CFG_FILE)" -ex "load"
endif
```
完成上述所有操作后，就可以直接在工程目录下运行`make run`命令实现编译和烧录

## 软件编写
### OLED驱动
#### 硬件部分
##### OLED模块
* 主控芯片为SSD1780，采用I2C通讯模式
 * 依据芯片手册，Slave Address=`7'b011_110(SA0)`
 * I2C通讯模式下，引脚`D/C`充当I2C地址低位`SA0`
* 屏幕大小0.96寸，长宽128*64
* 从模块的原理图可知，通过修改电阻焊接的位置可以修改`D/C`引脚的电平
* 用万用表直接测量后发现，屏幕的15号引脚`D/C`通过10欧姆电阻拉低到GND，故I2C模式下的地址为`7'b011_1100`也就是`0x3C`



