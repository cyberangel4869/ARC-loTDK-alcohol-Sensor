# ARC-loTDK-alcohol-Sensor
基于ARC loTDK开发板的嵌入式大作业，最终目标是用酒精浓度传感器感知酒精蒸汽的浓度，然后在ARC芯片处理后显示在OLED屏幕上
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

