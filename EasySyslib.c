/*
 * EasySys.c
 *
 *  Created on: Feb 18, 2024
 *      Author: wang_
 */



//#include <Device_V1.h>
#include <stdint.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <main.h>
//mylib
#include <EasySyslib.h>

//#include <Device_V1.h>		//HSPY PS, VC DMM, code lib
//#include <EasyPower_V1.h>	//INA226 DCMeter code lib
#include <OLED.h>			//0.9inch/SSD1306/128*32	1.3inch/SH1106/128*64	1.5inch/SH1107/128*128		lib
#include <USBPD.h>			//HUSB238 PD requst code

#include "stm32g0xx_hal.h"
#include "stm32g0xx_hal_crc.h"


//sys time
extern int8_t Enp_YEAR, Enp_MONTH, Enp_DAY, Enp_HOUR, Enp_MINUTE, Enp_SECOND;

extern uint8_t dateTimeBuffer[7];

//config
extern int8_t BoardID;
extern bool SW1_1, SW1_2, SW2_1, SW2_2;

//input
extern bool SW_UP, SW_DOWN, SW_LEFT, SW_RIGHT,SW_UP0, SW_DOWN0, SW_LEFT0, SW_RIGHT0,
	SW_UPUP, SW_DOWNDOWN, SW_LEFTLEFT, SW_RIGHTRIGHT,SW_UPUP0, SW_DOWNDOWN0, SW_LEFTLEFT0, SW_RIGHTRIGHT0;

//state
extern int sys_state; //0 - 6; 	0 - none; 1- green	;2 - cyan(青色，天蓝）;3 - blue	;4 - yellow	;5 - purple	;6 - red
extern bool ERR;


//// ////ccommunication
extern uint32_t len;
//CAN
extern uint8_t CANCTRLMSG[8];
extern uint8_t CH1RxData[8], CH2RxData[8];
extern uint8_t CH1TxData[8], CH2TxData[8];

extern int PS1_SET_V_H,PS1_SET_A_H,PS2_SET_V_H,PS2_SET_A_H,PS3_SET_V_H,PS3_SET_A_H;
extern int PS1_SET_V_L,PS1_SET_A_L,PS2_SET_V_L,PS2_SET_A_L,PS3_SET_V_L,PS3_SET_A_L;
extern uint16_t PS1_Ack_V,PS1_Ack_A,PS2_Ack_V,PS2_Ack_A,PS3_Ack_V,PS3_Ack_A;
extern bool PS1_SET_OF,PS2_SET_OF,PS3_SET_OF;
extern bool PS1_Ack_OF,PS2_Ack_OF,PS3_Ack_OF;
extern bool PS1_KL15,PS2_KL15,PS3_KL15,PS1_KL30,PS2_KL30,PS3_KL30;

//UART
extern uint8_t u1RxData[REC_LENGTH], u2RxData[REC_LENGTH], u3RxData[REC_LENGTH],
		u4RxData[REC_LENGTH], u5RxData[REC_LENGTH], u6RxData[REC_LENGTH],
		RxData[REC_LENGTH];
extern uint8_t utxData[] ;
extern uint8_t HSPYRXDATA[REC_LENGTH];
extern uint8_t Set_HSPY[9];
extern uint8_t Read_HSPY[6];
extern bool CAN1Rxflag,CAN2Rxflag;
extern uint8_t U1RXFlag , U2RXFlag , U3RXFlag , U4RXFlag , U5RXFlag ,U6RXFlag;// �????????????????????????????????????????????????????0时，忽略对应串口的接收数据，不处理中断内部函�????????????????????????????????????????????????????; 在需要接收串口数据是置为1

//I2C
extern uint8_t I2CRxData[REC_LENGTH];


////	stm32 header
extern FDCAN_TxHeaderTypeDef FDCAN1_TxHeader, FDCAN2_TxHeader;
extern FDCAN_RxHeaderTypeDef FDCAN1_RxHeader, FDCAN2_RxHeader;
extern FDCAN_FilterTypeDef FDCAN_Filter1;
extern FDCAN_FilterTypeDef FDCAN_Filter2;//
extern FDCAN_FilterTypeDef FDCAN_Filter; //

extern I2C_HandleTypeDef hi2c1;
//extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;

extern CRC_HandleTypeDef hcrc;	//CRC

extern  UART_HandleTypeDef huart1;
extern	UART_HandleTypeDef huart2;
extern	UART_HandleTypeDef huart3;
extern	UART_HandleTypeDef huart4;
extern	UART_HandleTypeDef huart5;
extern	UART_HandleTypeDef huart6;



void Order_MinF(uint8_t length,uint8_t * A)	//min 2 front
{
	uint8_t i,j,min,pos;

	for(i=0;i< length;i++)
	{
		min = A[i];
		pos = i;
		for(j = i+1;j< length;j++)
		{
			if (min>A[j])
			{
				min = A[j];
				pos = j;
			}
		}
		A[pos] = A[i];
		A[i] = min;
	}
}



////串口输出函数
//void uprint_byte(UART_HandleTypeDef *huart, uint8_t Data) {
//	//UART_HandleTypeDef uch;
//	sprintf(utxData, "%c", Data);
//	HAL_UART_Transmit(huart, (uint8_t*)utxData, strlen(0xf), 1);
//
////	if (ch == 1) {
////		sprintf(utxData, "%c", Data);
////		HAL_UART_Transmit(&huart1, (uint8_t*)utxData, strlen(0xf), 1);
////	} else if (ch == 2) {
////		sprintf(utxData, "%c", Data);
////		HAL_UART_Transmit(&huart2, (uint8_t*)utxData, strlen(0xf), 1);
////	} else if (ch == 3) {
//////		sprintf(utxData, "%c", Data);
//////		HAL_UART_Transmit(&huart3, (uint8_t*)utxData, strlen(0xf), 1);
////	} else if (ch == 4) {
//////		sprintf(utxData, "%c", Data);
//////		HAL_UART_Transmit(&huart4, (uint8_t*)utxData, strlen(0xf), 1);
////	} else if (ch == 5) {
////		sprintf(utxData, "%c", Data);
////		HAL_UART_Transmit(&huart5, (uint8_t*)utxData, strlen(0xf), 1);
////	} else if (ch == 6) {
////		sprintf(utxData, "%c", Data);
////		HAL_UART_Transmit(&huart6, (uint8_t*)utxData, strlen(0xf), 1);
////	}
//}
//
////串口输出函数,附带CRC
//void uprint_STR(UART_HandleTypeDef *huart, uint8_t *Data, int length) {
//	for (int i = 0; i < length; i++) {
//		uprint_byte(huart, Data[i]);
//		//HAL_Delay(1);
//	}
//}
//
//
//
//
////串口输出函数,附带CRC
//void uprint_CRC(int ch, uint8_t *Data, int length) {
//
//	uint8_t HSPY_CMD[length + 2];
//
//	for (int i = 0; i < length; i++) {
//		HSPY_CMD[i] = Data[i];
//	}
//
//	HSPY_CMD[length] = HAL_CRC_Calculate(&hcrc, Data, length);//由主机计算得到CRC�???????????????????????????????????????????????????????????????????
//	HSPY_CMD[length + 1] = HAL_CRC_Calculate(&hcrc, Data, length) >> 8;	//由主机计算得到CRC�???????????????????????????????????????????????????????????????????
//
//	for (int i = 0; i < length + 2; i++) {
//		uprint_byte(ch, HSPY_CMD[i]);
//		//HAL_Delay(1);
//	}
//}





////	button check
void button_check()
{
//		SW_RIGHT_Pin|SW_LEFT_Pin|SW_DOWN_Pin;
//		GPIOC
//		SW_DOWN, SW_LEFT, SW_RIGHT;

		if (HAL_GPIO_ReadPin(GPIOC, SW_LEFT_Pin) == GPIO_PIN_SET)
		{
			/* 如果引脚为高电平，代表按键触�???????? */
			SW_LEFT0 = 1;
			if (SW_LEFT != SW_LEFT0 )	//
			{
				SW_LEFT = SW_LEFT0;
				Sys_shortbeep();
			}
		}
		else
		{
			/* 如果引脚为低电平 */
			SW_LEFT = 0;	SW_LEFT0 = 0;	SW_LEFTLEFT = 0;	SW_LEFTLEFT0 = 0;
		}

		if (HAL_GPIO_ReadPin(GPIOC, SW_RIGHT_Pin) == GPIO_PIN_SET)
		{
			/* 如果引脚为高电平，代表按键触�???????? */
			SW_RIGHT0 = 1;
			if (SW_RIGHT != SW_RIGHT0 )	//
			{
				SW_RIGHT = SW_RIGHT0;
				Sys_shortbeep();
			}
		}
		else
		{
			/* 如果引脚为低电平 */
			SW_RIGHT = 0;	SW_RIGHT0 = 0;SW_RIGHTRIGHT = 0;	SW_RIGHTRIGHT0 = 0;
		}

		if (HAL_GPIO_ReadPin(GPIOC, SW_DOWN_Pin) == GPIO_PIN_SET)
		{
			/* 如果引脚为高电平，代表按键触�???????? */
			SW_DOWN0 = 1;
			if (SW_DOWN != SW_DOWN0 )	//
			{
				SW_DOWN = SW_DOWN0;
				Sys_shortbeep();
			}
		}
		else
		{
			/* 如果引脚为低电平 */
			SW_DOWN = 0;	SW_DOWN0 = 0;	SW_DOWNDOWN = 0;	SW_DOWNDOWN0 = 0;
		}

}

void button_L_check()	///////////check button long push
{
//	SW_UPUP, SW_DOWNDOWN, SW_LEFTLEFT, SW_RIGHTRIGHT
		if (SW_LEFT & SW_LEFT0)
		{
			if(SW_LEFTLEFT0)
			{				SW_LEFTLEFT = 1;			}
			else{
				SW_LEFTLEFT0 = 1;	}
		}
		else
		{	SW_LEFTLEFT0 = 0;	SW_LEFTLEFT = 0;		}

		if (SW_RIGHT & SW_RIGHT0)
		{
			if(SW_RIGHTRIGHT0)
			{	SW_RIGHTRIGHT = 1;	}
			else{
			SW_RIGHTRIGHT0 = 1;	}
		}
		else
		{	SW_RIGHTRIGHT0 = 0;			SW_RIGHTRIGHT = 0;
		}
		if (SW_DOWN & SW_DOWN0 )
		{
			if(SW_DOWNDOWN0)
			{	SW_DOWNDOWN = 1;	}
			else{
				SW_DOWNDOWN0 = 1;	}
		}
		else
		{	SW_DOWNDOWN0 = 0;		SW_DOWNDOWN = 0;
		}

}

////	sys state LED updte
//void Sys_LED_update()
//{
////int sys_state; //0 - 6; 	0 - none; 1- green	;2 - cyan(青色，天蓝）;3 - blue	;4 - yellow	;5 - purple	;6 - red
//	sys_state = sys_state%7;
//	switch(sys_state)
//	{
//	case 0:		//none
//		HAL_GPIO_WritePin(GPIOB, IO_SYS_LED_B_Pin, 0);
//		HAL_GPIO_WritePin(GPIOE, IO_SYS_LED_G_Pin, 0);
//		HAL_GPIO_WritePin(GPIOE, IO_SYS_LED_R_Pin, 0);
//		break;
//	case 1:		//green
//		HAL_GPIO_WritePin(GPIOB, IO_SYS_LED_B_Pin, 0);
//		HAL_GPIO_WritePin(GPIOE, IO_SYS_LED_G_Pin, 1);
//		HAL_GPIO_WritePin(GPIOE, IO_SYS_LED_R_Pin, 0);
//		break;
//	case 2:		//cyan
//		HAL_GPIO_WritePin(GPIOB, IO_SYS_LED_B_Pin, 1);
//		HAL_GPIO_WritePin(GPIOE, IO_SYS_LED_G_Pin, 1);
//		HAL_GPIO_WritePin(GPIOE, IO_SYS_LED_R_Pin, 0);
//		break;
//	case 3:		//blue
//		HAL_GPIO_WritePin(GPIOB, IO_SYS_LED_B_Pin, 1);
//		HAL_GPIO_WritePin(GPIOE, IO_SYS_LED_G_Pin, 0);
//		HAL_GPIO_WritePin(GPIOE, IO_SYS_LED_R_Pin, 0);
//		break;
//	case 4:		//yellow
//		HAL_GPIO_WritePin(GPIOB, IO_SYS_LED_B_Pin, 0);
//		HAL_GPIO_WritePin(GPIOE, IO_SYS_LED_G_Pin, 1);
//		HAL_GPIO_WritePin(GPIOE, IO_SYS_LED_R_Pin, 1);
//		break;
//	case 5:		//purple
//		HAL_GPIO_WritePin(GPIOB, IO_SYS_LED_B_Pin, 1);
//		HAL_GPIO_WritePin(GPIOE, IO_SYS_LED_G_Pin, 0);
//		HAL_GPIO_WritePin(GPIOE, IO_SYS_LED_R_Pin, 1);
//		break;
//	case 6:		//red
//		HAL_GPIO_WritePin(GPIOB, IO_SYS_LED_B_Pin, 0);
//		HAL_GPIO_WritePin(GPIOE, IO_SYS_LED_G_Pin, 0);
//		HAL_GPIO_WritePin(GPIOE, IO_SYS_LED_R_Pin, 1);
//		break;
//	}
//}


////	beep
//void Sys_longbeep()
//{
//	HAL_GPIO_WritePin(GPIOB, IO_BEEP_Pin, 1);
//	HAL_Delay(100);
//	HAL_GPIO_WritePin(GPIOB, IO_BEEP_Pin, 0);
//}
//
//void Sys_beep()
//{
//	HAL_GPIO_WritePin(GPIOB, IO_BEEP_Pin, 1);
//	HAL_Delay(40);
//	HAL_GPIO_WritePin(GPIOB, IO_BEEP_Pin, 0);
//}
//
//void Sys_shortbeep()
//{
//	HAL_GPIO_WritePin(GPIOB, IO_BEEP_Pin, 1);
//	HAL_Delay(10);
//	HAL_GPIO_WritePin(GPIOB, IO_BEEP_Pin, 0);
//}



//void Sys_tune2()
//{
//	HAL_GPIO_WritePin(GPIOB, IO_BEEP_Pin, 1);
//	HAL_Delay(10);
//	HAL_GPIO_WritePin(GPIOB, IO_BEEP_Pin, 0);
//}
//
//void Sys_tune3()
//{
//	HAL_GPIO_WritePin(GPIOB, IO_BEEP_Pin, 1);
//	HAL_Delay(10);
//	HAL_GPIO_WritePin(GPIOB, IO_BEEP_Pin, 0);
//}


////	ERROR
void myerror()
{
//	char *str1 = "SYS ERROR!";
//	OLED_Fill(OLED_I2C_ch ,OLED_type, 0x00);
//	OLED_ShowString(OLED0_I2C_ch ,OLED0_type,0, 0, str1);
//
//	//// ////HardReset
//	HAL_GPIO_WritePin(GPIOD, IO_BEEP_Pin, 1);
//	HAL_Delay(10);
//	HAL_GPIO_WritePin(GPIOD, IO_BEEP_Pin, 0);
//	HAL_Delay(100);
//	HAL_GPIO_WritePin(GPIOD, IO_BEEP_Pin, 1);
//	HAL_Delay(10);
//	HAL_GPIO_WritePin(GPIOD, IO_BEEP_Pin, 0);
//	HAL_Delay(100);
//	HAL_GPIO_WritePin(GPIOD, IO_BEEP_Pin, 1);
//	HAL_Delay(10);
//	HAL_GPIO_WritePin(GPIOD, IO_BEEP_Pin, 0);
//	HAL_Delay(100);
	//// ////
}

void errorANDreset()
{
	myerror();

	char *str1 = "SYS Reset!";
	OLED_ShowString(OLED_I2C_ch ,OLED_type,0, 1, str1);
	HAL_Delay(1000);
}


////	MD5
#define ROTATELEFT(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/**
 * @desc: convert message and mes_bkp string into integer array and store them in w
 */
void md5_process_part1(uint32_t *w, unsigned char *message,	uint32_t *pos, uint32_t mes_len, const unsigned char *mes_bkp) {
	uint32_t i; // used in for loop

	for (i = 0; i <= 15; i++) {
		int32_t count = 0;
		while (*pos < mes_len && count <= 24) {
			w[i] += (((uint32_t) message[*pos]) << count);
			(*pos)++;
			count += 8;
		}
		while (count <= 24) {
			w[i] += (((uint32_t) mes_bkp[*pos - mes_len]) << count);
			(*pos)++;
			count += 8;
		}
	}
}

/**
 * @desc: start encryption based on w
 */
void md5_process_part2(uint32_t abcd[4], uint32_t *w, const uint32_t k[64], const uint32_t s[64]) {
	uint32_t i; // used in for loop

	uint32_t a = abcd[0];
	uint32_t b = abcd[1];
	uint32_t c = abcd[2];
	uint32_t d = abcd[3];
	uint32_t f = 0;
	uint32_t g = 0;

	for (i = 0; i < 64; i++) {
		if (i >= 0 && i <= 15) {
			f = (b & c) | ((~b) & d);
			g = i;
		} else if (i >= 16 && i <= 31) {
			f = (d & b) | ((~d) & c);
			g = (5 * i + 1) % 16;
		} else if (i >= 32 && i <= 47) {
			f = b ^ c ^ d;
			g = (3 * i + 5) % 16;
		} else if (i >= 48 && i <= 63) {
			f = c ^ (b | (~d));
			g = (7 * i) % 16;
		}
		uint32_t temp = d;
		d = c;
		c = b;
		b = ROTATELEFT((a + f + k[i] + w[g]), s[i]) + b;
		a = temp;
	}

	abcd[0] += a;
	abcd[1] += b;
	abcd[2] += c;
	abcd[3] += d;
}

static const uint32_t k_table[] =
{ 0xd76aa478, 0xe8c7b756, 0x242070db,
		0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501, 0x698098d8,
		0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e,
		0x49b40821, 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d,
		0x02441453, 0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6, 0xc33707d6, 0xf4d50d87,
		0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a, 0xfffa3942,
		0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60,
		0xbebfbc70, 0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039,
		0xe6db99e5, 0x1fa27cf8, 0xc4ac5665, 0xf4292244, 0x432aff97, 0xab9423a7,
		0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1, 0x6fa87e4f,
		0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb,
		0xeb86d391 };

static const uint32_t s_table[] = { 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
		7, 12, 17, 22, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
		4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 6, 10, 15,
		21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21 };

int32_t cal_md5(unsigned char *result, unsigned char *data, int length)
{
	if (result == NULL)
	{		return 1;	}

	uint32_t w[16];
	uint32_t i; // used in for loop

	uint32_t mes_len = length;
	uint32_t looptimes = (mes_len + 8) / 64 + 1;
	uint32_t abcd[] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476 };

	uint32_t pos = 0; // position pointer for message
	uint32_t bkp_len = 64 * looptimes - mes_len; // 经过计算发现不超�????????????????????????????????????????????????????????????????????????????????????72

//    unsigned char *bkp_mes = (unsigned char *)calloc(1, bkp_len);
	unsigned char bkp_mes[80];
	for (int i = 0; i < 80; i++) //初始�????????????????????????????????????????????????????????????????????????????????????
			{		bkp_mes[i] = 0;	}

	bkp_mes[0] = (unsigned char) (0x80);
	uint64_t mes_bit_len = ((uint64_t) mes_len) * 8;
	for (i = 0; i < 8; i++)
	{
		bkp_mes[bkp_len - i - 1] = (unsigned char) ((mes_bit_len & (0x00000000000000FF << (8 * (7 - i)))) >> (8 * (7 - i)));
	}

	for (i = 0; i < looptimes; i++)
	{
		for (int j = 0; j < 16; j++) //初始�????????????????????????????????????????????????????????????????????????????????????
				{			w[j] = 0x00000000;		}

		md5_process_part1(w, data, &pos, mes_len, bkp_mes); // compute w
		md5_process_part2(abcd, w, k_table, s_table); // calculate md5 and store the result in abcd
	}

	for (int i = 0; i < 16; i++)
	{		result[i] = ((unsigned char*) abcd)[i];	}

	return 0;
}



//////////////////////////
//////////////W25Q64 SPI flash

uint8_t flash_Data_buffer[4096] = {0};
#define SectorBuf flash_Data_buffer


uint16_t SPI_Flash_GetID(void)
{
	uint16_t id = 0;
	uint8_t RxData,cmd;

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 0);
	HAL_Delay(1);

	cmd = Flash_Read_DeviceID;
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);

	cmd = 0x00;
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);

	cmd = 0xff;
	for (int i=0;i<2;i++)
	{
		HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
		id = id <<8;
		id = id + RxData;
	}

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 1);
	return id;
}


uint64_t SPI_Flash_GUID(void)
{
	uint64_t id = 0;
	uint8_t RxData,cmd;

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 0);
	HAL_Delay(1);

	cmd = Flash_Read_UniqueID;
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);

	cmd = 0x00;
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);

	cmd = 0xff;
	for (int i=0;i<8;i++)
	{
		HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
		id = id <<8;
		id = id + RxData;
	}

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 1);
	return id;
}

void SPI_Flash_WtritEnable(void)
{
	uint8_t cmd,RxData;
	cmd = Flash_Write_Enable_CMD;

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 0);
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 1);
}

void SPI_Flash_WtritDisable(void)
{
	uint8_t cmd,RxData;
	cmd = Flash_Write_Disable_CMD;

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 0);
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 1);
}

uint8_t SPI_Flash_ReadSR(void)
{
	uint8_t pdata = 0;
	uint8_t RxData,cmd;

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 0);
	HAL_Delay(1);

	cmd = Flash_Read_SR_CMD;
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);

//	cmd = 0x00;
//	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);

	cmd = 0xff;
	for (int i=0;i<1;i++)
	{
		HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
		pdata = pdata <<8;
		pdata = pdata + RxData;
	}

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 1);
	return pdata;
}

void SPI_Flash_WaitNoBusy(void)
{
	while( (SPI_Flash_ReadSR()&0x01) == 0x01 );
}

void SPI_Flash_WriteSR(uint8_t write_data)
{
	SPI_Flash_WtritEnable();
	SPI_Flash_WaitNoBusy();

	uint8_t TxData,RxData,cmd;
	TxData = write_data;

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 0);

	cmd = Flash_Write_SR_CMD;
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);

	HAL_SPI_TransmitReceive(Flash_SPI, &TxData, &RxData, 1,10);

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 1);
}

void SPI_Flash_ReadBytes(uint8_t *Rxpbuffer, uint32_t ReadAddr, uint16_t NumbBytes)
{
	uint8_t RxData,cmd,tmpbuff[3];
	cmd = Flash_Read_Data;

	tmpbuff[0] = (uint8_t)((ReadAddr>>16)&0xff);
	tmpbuff[1] = (uint8_t)((ReadAddr>>8)&0xff);
	tmpbuff[2] = (uint8_t)((ReadAddr>>0)&0xff);

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 0);

	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[0], &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[1], &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[2], &RxData, 1,10);

	uint8_t cmds[4096] = {0xff};
	HAL_SPI_TransmitReceive(Flash_SPI, cmds, Rxpbuffer, NumbBytes,10);

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 1);
}

void SPI_Flash_FastReadBytes(uint8_t *Rxpbuffer, uint32_t ReadAddr, uint16_t NumbBytes)
{
	uint8_t RxData,cmd,tmpbuff[3];
	cmd = Flash_FastRead_Data;

	tmpbuff[0] = (uint8_t)((ReadAddr>>16)&0xff);
	tmpbuff[1] = (uint8_t)((ReadAddr>>8)&0xff);
	tmpbuff[2] = (uint8_t)((ReadAddr>>0)&0xff);

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 0);

	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[0], &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[1], &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[2], &RxData, 1,10);

	uint8_t cmds[4096] = {0xff};
	HAL_SPI_TransmitReceive(Flash_SPI, cmds, Rxpbuffer, NumbBytes,10);

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 1);
}

void SPI_Flash_WritePage(uint8_t *Txpbuffer, uint32_t WrtieAddr, uint16_t NumbBytes)
{
	uint8_t RxData,cmd,tmpbuff[3];
	cmd = Flash_Write_Page;

	tmpbuff[0] = (uint8_t)((WrtieAddr>>16)&0xff);
	tmpbuff[1] = (uint8_t)((WrtieAddr>>8)&0xff);
	tmpbuff[2] = (uint8_t)((WrtieAddr>>0)&0xff);

	SPI_Flash_WtritEnable();
	SPI_Flash_WaitNoBusy();

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 0);

	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[0], &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[1], &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[2], &RxData, 1,10);

	uint8_t cmds[4096] = {0xff};
	HAL_SPI_TransmitReceive(Flash_SPI, Txpbuffer, cmds, NumbBytes,10);

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 1);
	SPI_Flash_WaitNoBusy();
}

void SPI_Flash_WriteNoCheck(uint8_t *Txpbuffer, uint32_t WrtieAddr, uint16_t NumbBytes)
{
	uint16_t PageByte = 256 - WrtieAddr%256 ;	//dangqian page shengyu kexie bytes
	if (NumbBytes <= PageByte)
	{
		PageByte = NumbBytes;
	}

	while(1)
	{
		SPI_Flash_WritePage(Txpbuffer, WrtieAddr, PageByte);
		if (NumbBytes == PageByte)
		{
			break;
		}else
		{
			Txpbuffer = PageByte + Txpbuffer;
			WrtieAddr = PageByte + WrtieAddr;
			NumbBytes = NumbBytes - PageByte;
			if(NumbBytes >256)
			{
				PageByte = 256;
			}else
			{
				PageByte = NumbBytes;
			}
		}
	}
}

void SPI_Flash_WriteSomeBytes(uint8_t *Txpbuffer, uint32_t WrtieAddr, uint16_t NumbBytes)
{
	uint32_t ulSecPos = 0;	//shanqu weizhi
	uint16_t usSecOff = 0;	//shanqu pianyi
	uint16_t usSecRemain = 0;	//shengyu shanqu
	uint32_t i = 0;

	ulSecPos = WrtieAddr/ 4096;
	usSecOff = WrtieAddr% 4096;
	usSecRemain = 4096 - usSecOff;

	if (NumbBytes <= usSecRemain)
	{
		usSecRemain = NumbBytes;
	}

	while(1)
	{
		SPI_Flash_ReadBytes(SectorBuf , ulSecPos*4096 , 4096 );
		for (i = 0; i< usSecRemain ; i++)
		{
			if (SectorBuf[usSecOff + i] != 0xff)
				break;
		}

		if (i < usSecRemain)	//need to erase
		{
			SPI_Flash_EraseSector(ulSecPos);	//erase
			for(i = 0; i < usSecRemain; i++)		//write
			{
				SectorBuf[usSecOff + i] = Txpbuffer[i];
			}
			SPI_Flash_WriteNoCheck(SectorBuf,ulSecPos*4096, 4096 );
		}
		else
		{
			SPI_Flash_WriteNoCheck(Txpbuffer, WrtieAddr, usSecRemain );
		}

		if(NumbBytes == usSecRemain)
		{
			SPI_Flash_WtritDisable();
			break;
		}
		else
		{
			ulSecPos++;
			usSecOff = 0;
			Txpbuffer = Txpbuffer + usSecRemain;
			WrtieAddr = WrtieAddr + usSecRemain;
			NumbBytes = NumbBytes - usSecRemain;

			if(NumbBytes > 4096)
			{
				usSecRemain = 4096;
			}
			else
			{
				usSecRemain = NumbBytes;
			}
		}
	}
}

void SPI_Flash_ErasePage(uint32_t pPageAddr)
{
	uint32_t WrtieAddr = pPageAddr *256;
	uint8_t RxData,cmd,tmpbuff[3];
	cmd = Flash_Erase_Page;
	tmpbuff[0] = (uint8_t)((WrtieAddr>>16)&0xff);
	tmpbuff[1] = (uint8_t)((WrtieAddr>>8)&0xff);
	tmpbuff[2] = (uint8_t)((WrtieAddr>>0)&0xff);

	SPI_Flash_WtritEnable();
	SPI_Flash_WaitNoBusy();

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 0);
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[0], &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[1], &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[2], &RxData, 1,10);
	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 1);

	SPI_Flash_WaitNoBusy();
}

void SPI_Flash_EraseSector(uint32_t pSectorAddr)
{
	uint8_t RxData,cmd,tmpbuff[3];
	cmd = Flash_Erase_Sector;
	uint32_t WrtieAddr = pSectorAddr * 4096;

	tmpbuff[0] = (uint8_t)((WrtieAddr>>16)&0xff);
	tmpbuff[1] = (uint8_t)((WrtieAddr>>8)&0xff);
	tmpbuff[2] = (uint8_t)((WrtieAddr>>0)&0xff);

	SPI_Flash_WtritEnable();
	SPI_Flash_WaitNoBusy();

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 0);
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[0], &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[1], &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[2], &RxData, 1,10);
	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 1);

	SPI_Flash_WaitNoBusy();
}

void SPI_Flash_EraseBlock(uint32_t pBlockAddr)
{
	uint8_t RxData,cmd,tmpbuff[3];
	cmd = Flash_Erase_Block;
	uint32_t WrtieAddr = pBlockAddr * 65536;

	tmpbuff[0] = (uint8_t)((WrtieAddr>>16)&0xff);
	tmpbuff[1] = (uint8_t)((WrtieAddr>>8)&0xff);
	tmpbuff[2] = (uint8_t)((WrtieAddr>>0)&0xff);

	SPI_Flash_WtritEnable();
	SPI_Flash_WaitNoBusy();

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 0);
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[0], &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[1], &RxData, 1,10);
	HAL_SPI_TransmitReceive(Flash_SPI, &tmpbuff[2], &RxData, 1,10);
	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 1);

	SPI_Flash_WaitNoBusy();
}

void SPI_Flash_EraseChip(void)
{
	uint8_t RxData,cmd;
	cmd = Flash_Erase_Chip;

	SPI_Flash_WtritEnable();
	SPI_Flash_WaitNoBusy();

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 0);
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 1);

	SPI_Flash_WaitNoBusy();
}

void SPI_Flash_PowerDown(void)
{
	uint8_t RxData,cmd;
	cmd = Flash_Power_Down;

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 0);
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 1);

	HAL_Delay(3);
}

void SPI_Flash_PowerUp(void)
{
	uint8_t RxData,cmd;
	cmd = Flash_Release_Power_Down;

	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 0);
	HAL_SPI_TransmitReceive(Flash_SPI, &cmd, &RxData, 1,10);
	HAL_GPIO_WritePin(SPI_Flash_NSS_Group, SPI_Flash_NSS_Pin, 1);

	HAL_Delay(3);
}







//void PCF8563_ReadDateTime(uint8_t *buffer)
//{
//HAL_I2C_Mem_Read(&hi2c1, PCF8563_ADDRESS, 2, I2C_MEMADD_SIZE_8BIT, buffer, 7, HAL_MAX_DELAY);
//}





void XL9555_Init(uint8_t addr,uint8_t port1,uint8_t port2)
{
	uint8_t port0_config, port1_config;

	if(port1 == 0)
	{
		port0_config = 0x00;
	}else
	{
		port0_config = 0xff;
	}
	HAL_I2C_Mem_Write(&hi2c3,addr,0x06,1,&port0_config,1,1);	//config xl9555_port0

	if(port2 == 0)
	{
		port1_config = 0x00;
	}else
	{
		port1_config = 0xff;
	}
	HAL_I2C_Mem_Write(&hi2c3,addr,0x07,1,&port1_config,1,1);	//config xl9555_port1

}

uint8_t XL9555_Read(uint8_t addr,uint8_t port)	//port:0 or 1
{
	uint8_t XL9555_inputRead;

	if(port == 0)
	{
		HAL_I2C_Mem_Read(&hi2c3, addr, 0x00, 1,&XL9555_inputRead, 1, HAL_MAX_DELAY);
	}
	else if(port == 1)
	{
		HAL_I2C_Mem_Read(&hi2c3, addr, 0x01, 1,&XL9555_inputRead, 1, HAL_MAX_DELAY);
	}

	return XL9555_inputRead;
}


void TMP1075_Init(uint8_t ch, uint8_t addr)
{
	uint8_t Init1075[1];
	Init1075[0] = 0x60;

	if (ch == 1)
	{
		HAL_I2C_Mem_Write(&hi2c1, addr, TMP1075_CONFIG_REG, I2C_MEMADD_SIZE_8BIT, Init1075, sizeof(Init1075),10);
	}
	else if (ch == 2)
	{
//		HAL_I2C_Mem_Write(&hi2c2, addr, TMP1075_CONFIG_REG, I2C_MEMADD_SIZE_8BIT, Init1075, sizeof(Init1075),10);
	}
	else if (ch == 3)
	{
		HAL_I2C_Mem_Write(&hi2c3, addr, TMP1075_CONFIG_REG, I2C_MEMADD_SIZE_8BIT, Init1075, sizeof(Init1075),10);
	}
}

float readTemperature(uint8_t ch, uint8_t addr)
{
	uint8_t Data1075[2];
	uint16_t rawData = 0;
	Data1075[0] = 0;
	Data1075[1] = 0;

	if (ch == 1)
	{
		HAL_I2C_Mem_Read(&hi2c1, addr, TMP1075_TEMP_REG, 1, Data1075, 2, HAL_MAX_DELAY);
	}
	else if (ch == 2)
	{
//		HAL_I2C_Mem_Read(&hi2c2, addr, TMP1075_TEMP_REG, 1, Data1075, 2, HAL_MAX_DELAY);
	}
	else if (ch == 3)
	{
		HAL_I2C_Mem_Read(&hi2c3, addr, TMP1075_TEMP_REG, 1, Data1075, 2, HAL_MAX_DELAY);
	}

	rawData = (Data1075[0] << 4) | (Data1075[1] >> 4);
	if (rawData & 0x800)
	{
		rawData |= 0xF000; // 扩展符号位至16位（0xF000 = 1111000000000000）
	}
	float temperature = rawData * 0.0625;

	return temperature;

}





