# Application name
APPL ?= alcoholSensor

#
# root dir of embARC
#
EMBARC_ROOT = ../..

MID_SEL = common

# application source dirs
APPL_CSRC_DIR = ./src  ./lib
APPL_ASMSRC_DIR = .

# application include dirs
APPL_INC_DIR = ./src  ./lib  ../../board/iotdk  ../../include/device

# include current project makefile
COMMON_COMPILE_PREREQUISITES += makefile

# use ARC GNU to complie
TOOLCHAIN ?= gnu

# the board is IoTDK
BOARD ?= iotdk

### Options above must be added before include options.mk ###
# include key embARC build system makefile
include $(EMBARC_ROOT)/options/options.mk
