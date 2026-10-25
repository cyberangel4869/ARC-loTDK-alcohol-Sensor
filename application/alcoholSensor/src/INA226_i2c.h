#include "embARC.h"
#include "embARC_debug.h"
#include "device/ip_hal/dev_iic.h"
#include "iotdk/iotdk.h"

/* INA226 配置 */
#define INA226_IIC_ID       DFSS_IIC_0_ID
#define INA226_ADDR         0x40        /* 7位地址，视A0/A1引脚而定 */
#define INA226_REG_CONFIG   0x00
#define INA226_REG_BUS_VOLT 0x02

/* 配置值：连续测量总线电压，转换时间1.1ms，不平均 */
/* 位域：[15]RST=0 [14:12]保留 [11:9]AVG=000 [8:6]VBUSCT=100(1.1ms) [5:3]VSHCT=100 [2:0]MODE=111(连续) */
#define INA226_CONFIG_VAL   0x4127      /* TI 默认值，连续测量 shunt+bus */

static DEV_IIC_PTR ina226_iic;

/* 复用你已验证的 I2C 写寄存器函数 */
static int32_t ina226_write_reg(uint8_t reg, uint16_t val)
{
    uint8_t buf[2];
    int32_t ercd;
    buf[0] = (val >> 8) & 0xFF;
    buf[1] = val & 0xFF;

    ina226_iic->iic_control(IIC_CMD_MST_SET_TAR_ADDR, CONV2VOID(INA226_ADDR));
    ina226_iic->iic_control(IIC_CMD_MST_SET_NEXT_COND, CONV2VOID(IIC_MODE_RESTART));
    ercd = ina226_iic->iic_write(&reg, 1);
    if (ercd != 1) return -1;
    ina226_iic->iic_control(IIC_CMD_MST_SET_NEXT_COND, CONV2VOID(IIC_MODE_STOP));
    ercd = ina226_iic->iic_write(buf, 2);
    return (ercd == 2) ? 0 : -1;
}

/* 复用你已验证的 I2C 读寄存器函数 */
static int32_t ina226_read_reg(uint8_t reg, uint16_t *val)
{
    uint8_t buf[2];
    int32_t ercd;

    ina226_iic->iic_control(IIC_CMD_MST_SET_TAR_ADDR, CONV2VOID(INA226_ADDR));
    ina226_iic->iic_control(IIC_CMD_MST_SET_NEXT_COND, CONV2VOID(IIC_MODE_RESTART));
    ercd = ina226_iic->iic_write(&reg, 1);
    if (ercd != 1) return -1;
    ina226_iic->iic_control(IIC_CMD_MST_SET_NEXT_COND, CONV2VOID(IIC_MODE_STOP));
    ercd = ina226_iic->iic_read(buf, 2);//INA226的寄存器是16位的
    if (ercd != 2) return -1;
    *val = ((uint16_t)buf[0] << 8) | buf[1];
    return 0;
}

/* 初始化 INA226：打开 I2C 并写入配置 */
int ina226_init(void)
{
    int32_t ercd;
    ina226_iic = iic_get_dev(INA226_IIC_ID);
    /*检查i2c0是否已经配置*/
    if(ina226_iic==NULL)return -1;
    ercd = ina226_iic->iic_open(DEV_MASTER_MODE, IIC_SPEED_STANDARD);
    if (ercd != E_OK && ercd != E_OPNED) return -1;
    board_delay_ms(10, OSP_DELAY_OS_COMPAT_DISABLE);
    /* 写入配置寄存器，启动连续测量 */
    return ina226_write_reg(INA226_REG_CONFIG, INA226_CONFIG_VAL);
}

/* 轮询读取总线电压（单位：伏特） */
float ina226_read_bus_voltage(void)
{
    uint16_t raw;
    if (ina226_read_reg(INA226_REG_BUS_VOLT, &raw) != 0)
        return -1.0f;
    /* 总线电压 LSB = 1.25 mV */
    return (float)raw * 1.25f;
}
