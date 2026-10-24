#include "u8g2.h"

#include "embARC.h"
#include "embARC_debug.h"
#include "device/ip_hal/dev_iic.h"
#include "iotdk/iotdk.h"

#define OLED_IIC_ID     DFSS_IIC_0_ID   // 你的 I2C0 设备 ID
#define OLED_IIC_ADDR   0x3C            // OLED从机地址

static DEV_IIC_PTR oled_iic = NULL;

static int32_t oled_iic_init(void)
{
    int32_t ercd;

    oled_iic = iic_get_dev(OLED_IIC_ID);
    if (oled_iic == NULL){
        EMBARC_PRINTF("iic_get_dev error,exit with -1");
        return -1;
    } 
    ercd = oled_iic->iic_open(DEV_MASTER_MODE, IIC_SPEED_STANDARD);
    if (ercd != E_OK && ercd != E_OPNED){
        EMBARC_PRINTF("iic_open return %d",ercd);
        return ercd;
    } 
    board_delay_ms(10, OSP_DELAY_OS_COMPAT_DISABLE);
    EMBARC_PRINTF("iic0 Init Successful\n");
    return E_OK;
}

static uint8_t buf[128];
static uint16_t buf_len;

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


void board_delay_us(uint32_t us)
{
    uint64_t start_us = board_get_cur_us();
    while ((board_get_cur_us() - start_us) < us) {
        ;
    }
}


uint8_t ARC_u8g2_HAL_gpio_dely_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr){
    switch (msg) {
    case U8X8_MSG_DELAY_MILLI:
        board_delay_ms(arg_int, 1);
        break;
    case U8X8_MSG_DELAY_10MICRO:
        board_delay_us(arg_int * 10);
        break;
    case U8X8_MSG_DELAY_100NANO:
        /*直接忽略，硬件I2C不需要ns级别的软延时*/
        break;
    default:
        break;
    }
    return 1;
}


// 在你的测试代码里，用这个函数替代你手写的发送逻辑
static int32_t test_mpu_write(uint32_t slaveaddr, uint8_t regaddr, uint8_t val)
{
    int32_t ercd;
    uint8_t data[1];
    DEV_IIC_PTR iic_obj = iic_get_dev(DFSS_IIC_0_ID);
    if (!iic_obj) return -1;

    data[0] = regaddr;

    iic_obj->iic_control(IIC_CMD_MST_SET_TAR_ADDR, CONV2VOID(slaveaddr));
    iic_obj->iic_control(IIC_CMD_MST_SET_NEXT_COND, CONV2VOID(IIC_MODE_RESTART));
    ercd = iic_obj->iic_write(data, 1);          // 发寄存器地址
    iic_obj->iic_control(IIC_CMD_MST_SET_NEXT_COND, CONV2VOID(IIC_MODE_STOP));
    ercd = iic_obj->iic_write(&val, 1);          // 发数据
    return ercd;
}
