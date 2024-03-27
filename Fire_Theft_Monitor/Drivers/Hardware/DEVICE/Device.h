/*
 * @Author: wangzihao 2281036574@qq.com 21041428
 * @Date: 2024-03-18 
 */
#ifndef __DEVICE_H
#define __DEVICE_H

#include "stm32f4xx_hal.h"
#include "DS_18B20.h"

#define KEY1 0X01
#define KEY2 0X02
#define KEY3 0X04
#define KEY4 0X08
#define KEY5 0X10
#define KEY6 0X20
#define KEYCNT 2


#define   L1     262-1//低调 do 的频率
#define   L2     294-1//低调 re 的频率
#define   L3     330-1//低调 mi 的频率
#define   L4     350-1//低调 fa 的频率
#define   L5     392-1//低调 sol 的频率
#define   L6     440-1//低调 la 的频率
#define   L7     494-1//低调 si 的频率
                                               
#define   M1     524-1//中调 do 的频率
#define   M2     588-1//中调 re 的频率
#define   M3     660-1//中调 mi 的频率
#define   M4     700-1//中调 fa 的频率
#define   M5     784-1//中调 sol 的频率
#define   M6     880-1//中调 la 的频率
#define   M7     988-1//中调 si 的频率
 
#define   H1     1048-1//高调 do 的频率
#define   H2     1176-1//高调 re 的频率
#define   H3     1320-1//高调 mi 的频率
#define   H4     1480-1//高调 fa 的频率
#define   H5     1640-1//高调 sol 的频率
#define   H6     1760-1//高调 la 的频率
#define   H7     1976-1//高调 si 的频率
 
#define   S      0//不发音

void Information_Update(void);
void MPU_6050data(void);
void DS18B20_TEMP(void);
void Warn_Count(void);
void SetLeds(uint8_t dat);
void Alarm_beep(uint8_t state);
void Beep(uint16_t time, uint8_t tune);
void BeepDone(void);
void Write595(uint8_t sel, uint8_t num, uint8_t bdot);
void DispSeg(uint8_t num[4], uint8_t dot); 
void Ulock(void);
uint8_t Ulock_scan(void);
uint8_t key_scan(void);
uint8_t ScanKey(void);

#endif