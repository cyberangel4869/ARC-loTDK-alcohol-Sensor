/* ------------------------------------------
 * Copyright (c) 2017, Synopsys, Inc. All rights reserved.

 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:

 * 1) Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.

 * 3) Neither the name of the Synopsys, Inc., nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
   --------------------------------------------- */

#include "embARC.h"
#include "embARC_debug.h"
#include "u8g2_hal_embarc.h"


/**
 * @brief	Test hardware board without any peripheral
 */

 
int main(void)
{

	u8g2_t u8g2;
	//初始化u8g2库，传入u8g2数据体，旋转配置和HAL硬件抽象层回调函数的指针
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2,U8G2_R0,ARC_u8g2_HAL_msg_cb,ARC_u8g2_HAL_gpio_dely_cb);
	//通过I2C通讯复位OLED屏幕
	u8g2_InitDisplay(&u8g2);
	u8g2_ClearBuffer(&u8g2);
	board_delay_ms(100,0);
	//唤醒屏幕
	u8g2_SetPowerSave(&u8g2,0);
	while (1) {
        u8g2_ClearBuffer(&u8g2);
		u8g2_SetFont(&u8g2,u8g2_font_7x14B_tr);//设置字体
		u8g2_SetFontMode(&u8g2,1);//采用透明字体
		u8g2_SetDrawColor(&u8g2,2);//字体点阵通过XOR叠加到背景中
		u8g2_DrawBox(&u8g2,0,0,128,15);//顶部填充页眉
        u8g2_DrawStr(&u8g2,42, 13, "Alcohol");
		u8g2_SetFont(&u8g2,u8g2_font_7x14_tr);
		u8g2_DrawStr(&u8g2,30,30,"50 ppm");
        u8g2_SendBuffer(&u8g2);
        board_delay_ms(500,0);
    }
	
}
