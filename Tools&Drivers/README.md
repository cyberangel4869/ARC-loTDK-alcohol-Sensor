# 开发工具链

## ARC GNU编译器

Github Release页面：[ARC GNU 2026](https://github.com/foss-for-synopsys-dwc-arc-processors/toolchain/releases)

解压后需要添加`bin`目录的地址到系统的`PATH`环境变量

## embARC OSP库函数

这个库包含了所有ARC系列芯片和开发板的库函数和宏定义，其中包含大量的示例程序，以压缩包的形式放在`./embarc_osp.7z`中

解压后放在任意地址均可，但注意将自己的项目放在`embarc_osp/application`中，避免和官方的示例程序混在一起
