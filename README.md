# ARC-loTDK-alcohol-Sensor
基于ARC loTDK开发板的嵌入式大作业，最终目标是用酒精浓度传感器感知酒精蒸汽的浓度，然后在ARC芯片处理后显示在OLED屏幕上
## 硬件资源
### MQ-3酒精浓度传感器
##### 驱动方式：ADC直接读取输出电压
* 软件上只需要配置一路ADC外加一个定时器
* 查询MQ-3的传感器手册并线性拟合输出电压和酒精浓度的函数
$ lg(PPM)=(lg(R_s/R_0)-b)/m $
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

