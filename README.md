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
#### 硬件层：OLED模块
* 主控芯片为SSD1780，采用I2C通讯模式
  * 依据芯片手册，Slave Address=`7'b011_110(SA0)`
  * I2C通讯模式下，引脚`D/C`充当I2C地址低位`SA0`
* 屏幕大小0.96寸，长宽128*64
* 从模块的原理图可知，通过修改电阻焊接的位置可以修改`D/C`引脚的电平

用万用表直接测量后发现，屏幕的15号引脚`D/C`通过10欧姆电阻拉低到GND，故I2C模式下的地址为`7'b011_1100`也就是`0x3C`
#### 软件层：U8G2开源库

u8g2是应用最广泛的多平台OLED驱动库，它的标准库函数只提供了软件层，需要自行编写HAL硬件抽象层。

具体做法是用ARC标准库中的I2C硬件通讯驱动函数重写u8g2库的字节通信回调函数（处理 I2C 数据传输），GPIO回调函数和延时回调函数（处理时序）：
```c
//u8x8.h头文件

//通用函数指针原型
typedef uint8_t (*u8x8_msg_cb)(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

//字节通信回调函数指针与延时回调函数指针
struct u8x8_struct
{
  const u8x8_display_info_t *display_info;
  u8x8_char_cb next_cb; /*  procedure, which will be used to get the next char from the string */
  u8x8_msg_cb display_cb;
  u8x8_msg_cb cad_cb;
  u8x8_msg_cb byte_cb;
  u8x8_msg_cb gpio_and_delay_cb;
  uint32_t bus_clock;	/* can be used by the byte function to store the clock speed of the bus */
  const uint8_t *font;
  uint16_t encoding;		/* encoding result for utf8 decoder in next_cb */
  uint8_t x_offset;	/* copied from info struct, can be modified in flip mode */
  uint8_t is_font_inverse_mode; 	/* 0: normal, 1: font glyphs are inverted */
  uint8_t i2c_address;	/* a valid i2c adr. Initially this is 255, but this is set to something useful during DISPLAY_INIT */
					/* i2c_address is the address for writing data to the display */
					/* usually, the lowest bit must be zero for a valid address */
  uint8_t i2c_started;	/* for i2c interface */
  //uint8_t device_address;	/* OBSOLETE???? - this is the device address, replacement for U8X8_MSG_CAD_SET_DEVICE */
  uint8_t utf8_state;		/* number of chars which are still to scan */
  uint8_t gpio_result;	/* return value from the gpio call (only for MENU keys at the moment) */ 
  uint8_t debounce_default_pin_state;
  uint8_t debounce_last_pin_state;
  uint8_t debounce_state;
  uint8_t debounce_result_msg;	/* result msg or event after debounce */
#ifdef U8X8_WITH_USER_PTR
  void *user_ptr;
#endif
#ifdef U8X8_USE_PINS 
  uint8_t pins[U8X8_PIN_CNT];	/* defines a pinlist: Mainly a list of pins for the Arduino Environment, use U8X8_PIN_xxx to access */
#endif
};
```
用ARC库重写这些标准函数后，将函数名传递给u8g2初始化函数，就能实现u8g2和HAL层的对接
```c
void u8g2_Setup_ssd1306_128x64_noname_f(u8g2_t *u8g2, const u8g2_cb_t *rotation, u8x8_msg_cb byte_cb, u8x8_msg_cb gpio_and_delay_cb);
```
#### 硬件抽象层：HAL回调函数
OLED的I2C数据结构和常见的芯片不同，它没有“寄存器”的概念，指令字节和数据字节必须在“START->DATA->STOP“中一次性传递，这和常见的“START->REGaddr->RESTART->DATA->STOP"结构有很大的不同，且需要较大的I2C数据缓冲区。u8g2库的软件层通过向回调函数传递变量`uint8_t msg`控制回调函数的行为，回调函数中需要将软件层的`msg`行为和ARC自带的I2C驱动相对应，保证软件层可以正常的实现OLED通讯时序。因此本项目直接在回调函数文件`application/alocholSensor/lib/u8g2_hal_rmbarc.h`中利用变量`static uint8_t buf[128]`手动创建了一块静态的内存空间充当I2C通讯的数据缓冲区

u8g2的软件层在一次通讯中会先传入`MSG_BYTE_START_TRANSFR`指令开启通讯，然后多次传入`MSG_BYTE_SEND`指令并同时传入数据的指针，这些数据回先存入`buf`静态变量，最后在u8g2层传入`MSG_END_TRANSRF`结束通讯时调用ARC的`oled_iic->iic_write(uint_8,uint_8*)`函数一次性发送`buf`缓冲区中的所有数据，保证了通讯时许的正确

```c
// u8g2 HAL 回调
uint8_t ARC_u8g2_HAL_msg_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    uint8_t *data;
    int32_t ercd;

    switch (msg) {
    case U8X8_MSG_BYTE_INIT:
        // 首次通信前初始化 I2C0
        if (oled_iic_init() != E_OK) return 0;
        break;

    case U8X8_MSG_BYTE_START_TRANSFER:
        /* 设置从机地址 */
        oled_iic->iic_control(IIC_CMD_MST_SET_TAR_ADDR, CONV2VOID(OLED_IIC_ADDR));
        oled_iic->iic_control(IIC_CMD_MST_SET_NEXT_COND,CONV2VOID(IIC_MODE_STOP));
        break;

    case U8X8_MSG_BYTE_SEND:
        data = (uint8_t *)arg_ptr;
        if (buf_len + arg_int <= sizeof(buf)) {
            memcpy(buf + buf_len, data, arg_int);
            buf_len += arg_int;
        }
        break;

    case U8X8_MSG_BYTE_END_TRANSFER:
        if (buf_len > 0) {
            ercd = oled_iic->iic_write(buf, buf_len);
        if (ercd != buf_len) return 0;
            buf_len = 0;
        }
        #ifdef DEBUG
        EMBARC_PRINTF("TX len=%d: %02X %02X %02X %02X\r\n",
              buf_len, buf[0], buf_len>1?buf[1]:0,
              buf_len>2?buf[2]:0, buf_len>3?buf[3]:0);
        #endif
        break;

    case U8X8_MSG_BYTE_SET_DC:
        // I2C 接口没有 DC 引脚，忽略
        break;

    default:
        // 其他 GPIO/延时消息在硬件 I2C 下通常不需要
        return 1;
    }

    return 1;  // 成功
}
```
## 程序烧录
### 常见问题
#### 3333端口被占用导致烧录失败
OpenOCD进程需要手动结束，有可能出现旧的进程没有结束，占用通讯端口导致烧录失败的情况

解决方法：先检查3333端口占用情况或直接查找openocd进程
```bash
# 查看谁占用了 3333 端口
sudo lsof -i :3333
# 或者
sudo ss -tlnp | grep 3333
# 或者直接找 openocd 进程
ps aux | grep openocd
```
发现有残留进程直接杀掉
```bash
sudo kill -9 <PID> #这里的PID需要替换为真实的进程编号
```
或者直接
```bash
sudo pkill -9 openocd
```




