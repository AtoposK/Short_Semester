#include "DS_18B20.h"

 /**************************************************************************************
 * 描  述 : 微秒延时初始化
 * 入  参 : 无
 * 返回值 : 无
 **************************************************************************************/
void DWT_Init(void)
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;	// 使能DWT外设
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk ; 					// 使能Cortex-M DWT CYCCNT寄存器
	DWT->CYCCNT = 0; 																// DWT CYCCNT寄存器计数清0
}

 /**************************************************************************************
 * 描  述 : 微秒延时初始化
 * 入  参 : uint32_t
 * 返回值 : 无
 **************************************************************************************/
void delay_us(uint32_t us) {
		volatile uint32_t cycles = (SystemCoreClock / 1000000L) * us;
		volatile uint32_t start = DWT->CYCCNT;
		do  {
		} while(DWT->CYCCNT - start < cycles);
}

 /**************************************************************************************
 * 描  述 : 配置DS18B20用到的I/O口
 * 入  参 : 无
 * 返回值 : 无
 **************************************************************************************/
static void DS18B20_GPIO_Config(void)
{		
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOE_CLK_ENABLE();

  GPIO_InitStruct.Pin = DS18B20_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);
	
	DS18B20_DATA_OUT(1);	 //DS18B20数据引脚初始化配置为高电平输出
}

 /**************************************************************************************
 * 描  述 : 配置使DS18B20-DATA引脚变为输入模式
 * 入  参 : 无
 * 返回值 : 无
 **************************************************************************************/
static void DS18B20_Mode_IPU(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = DS18B20_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);
}

 /**************************************************************************************
 * 描  述 : 配置使DS18B20-DATA引脚变为输出模式
 * 入  参 : 无
 * 返回值 : 无
 **************************************************************************************/
static void DS18B20_Mode_Out_PP(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = DS18B20_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);
}

 /**************************************************************************************
 * 描  述 : 主机给从机发送复位脉冲
 * 入  参 : 无
 * 返回值 : 无
 **************************************************************************************/
static void DS18B20_Rst(void)
{
	DS18B20_Mode_Out_PP();     //主机设置为推挽输出 
	
	DS18B20_DATA_OUT(LOW);     //主机至少产生480us的低电平复位信号
	delay_us(750);
	DS18B20_DATA_OUT(HIGH);   //主机在产生复位信号后，需将总线拉高
	delay_us(15);   					//从机接收到主机的复位信号后，会在15~60us后给主机发一个存在脉冲
}

 /**************************************************************************************
 * 描  述 : 检测从机给主机返回的存在脉冲
 * 入  参 : 无
 * 返回值 : 0：成功   1：失败  
 **************************************************************************************/
static uint8_t DS18B20_Presence(void)
{
	uint8_t pulse_time = 0;
	
	DS18B20_Mode_IPU();    //主机设置为上拉输入
	
	/* 等待存在脉冲的到来，存在脉冲为一个60~240us的低电平信号 
	 * 如果存在脉冲没有来则做超时处理，从机接收到主机的复位信号后，会在15~60us后给主机发一个存在脉冲
	 */
	while( DS18B20_DATA_IN() && pulse_time<100 )
	{
		pulse_time++;
		delay_us(1);
	}	

	if( pulse_time >=100 )  //经过100us后，存在脉冲都还没有到来
		return 1;             //读取失败
	else                 		//经过100us后，存在脉冲到来
		pulse_time = 0;    		//清零计时变量
	
	while( !DS18B20_DATA_IN() && pulse_time<240 )  // 存在脉冲到来，且存在的时间不能超过240us
	{
		pulse_time++;
		delay_us(1);
	}	
	if( pulse_time >=240 ) // 存在脉冲到来，且存在的时间超过了240us
		return 1;            //读取失败
	else
		return 0;
}

 /**************************************************************************************
 * 描  述 : 从DS18B20读取一个bit
 * 入  参 : 无
 * 返回值 : uint8_t
 **************************************************************************************/
static uint8_t DS18B20_Read_Bit(void)
{
	uint8_t dat;
	
	/* 读0和读1的时间至少要大于60us */	
	DS18B20_Mode_Out_PP();
	/* 读时间的起始：必须由主机产生 >1us <15us 的低电平信号 */
	DS18B20_DATA_OUT(LOW);
	delay_us(10);
	
	/* 设置成输入，释放总线，由外部上拉电阻将总线拉高 */
	DS18B20_Mode_IPU();
	
	if( DS18B20_DATA_IN() == SET )
		dat = 1;
	else
		dat = 0;
	
	/* 这个延时参数请参考时序图 */
	delay_us(45);
	
	return dat;
}

 /**************************************************************************************
 * 描  述 : 从DS18B20读一个字节，低位先行
 * 入  参 : 无
 * 返回值 : uint8_t  
 **************************************************************************************/
uint8_t DS18B20_Read_Byte(void)
{
	uint8_t i, j, dat = 0;	
	
	for(i=0; i<8; i++) 
	{
		j = DS18B20_Read_Bit();		//从DS18B20读取一个bit
		dat = (dat) | (j<<i);
	}
	
	return dat;																																																																																
}

 /**************************************************************************************
 * 描  述 : 写一个字节到DS18B20，低位先行
 * 入  参 : uint8_t
 * 返回值 : 无  
 **************************************************************************************/
void DS18B20_Write_Byte(uint8_t dat)
{
	uint8_t i, testb;
	DS18B20_Mode_Out_PP();
	
	for( i=0; i<8; i++ )
	{
		testb = dat&0x01;
		dat = dat>>1;		
		/* 写0和写1的时间至少要大于60us */
		if (testb)
		{			
			DS18B20_DATA_OUT(LOW);
			delay_us(8);   //1us < 这个延时 < 15us
			
			DS18B20_DATA_OUT(HIGH);
			delay_us(58);    //58us+8us>60us
		}		
		else
		{			
			DS18B20_DATA_OUT(LOW);  
			/* 60us < Tx 0 < 120us */
			delay_us(70);
			
			DS18B20_DATA_OUT(HIGH);			
			/* 1us < Trec(恢复时间) < 无穷大*/
			delay_us(2);
		}
	}
}

 /**************************************************************************************
 * 描  述 : 起始DS18B20
 * 入  参 : 无
 * 返回值 : 无  
 **************************************************************************************/
void DS18B20_Start(void)
{
	DS18B20_Rst();	           //主机给从机发送复位脉冲
	DS18B20_Presence();	       //检测从机给主机返回的存在脉冲
	DS18B20_Write_Byte(0XCC);		 // 跳过 ROM 
	DS18B20_Write_Byte(0X44);		 // 开始转换
}

 /**************************************************************************************
 * 描  述 : DS18B20初始化函数
 * 入  参 : 无
 * 返回值 : uint8_t
 **************************************************************************************/
uint8_t ds18b20_init(void)
{
	DWT_Init();
	
	DS18B20_GPIO_Config();   
	DS18B20_Rst();
	
	return DS18B20_Presence();
}

 /**************************************************************************************
 * 描  述 : 从DS18B20读取温度值
 * 入  参 : 无  
 * 返回值 : float 
 **************************************************************************************/
float ds18b20_read(void)
{
	uint8_t tpmsb, tplsb;
	short s_tem;
	float f_tem;
	
	DS18B20_Rst();	   
	DS18B20_Presence();	 
	DS18B20_Write_Byte(0XCC);				/* 跳过 ROM */
	DS18B20_Write_Byte(0X44);				/* 开始转换 */
	
	DS18B20_Rst();
  DS18B20_Presence();
	DS18B20_Write_Byte(0XCC);				/* 跳过 ROM */
  DS18B20_Write_Byte(0XBE);				/* 读温度值 */
	
	tplsb = DS18B20_Read_Byte();
	tpmsb = DS18B20_Read_Byte();
	
	s_tem = tpmsb<<8;
	s_tem = s_tem | tplsb;
	
	if( s_tem < 0 )									/* 负温度 */
		f_tem = (~s_tem+1) * 0.0625;	
	else
		f_tem = (s_tem * 0.0625);	  
	return f_tem; 	
}

/*************************************END OF FILE******************************/