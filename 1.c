/*
 * @Author: Guoquan Wei 1940359148@qq.com 
 * @Date: 2018-05-03 21:03:48 
 * @Last Modified by: Guoquan Wei
 * @Last Modified time: 2018-05-03 22:35:01
 */

#include <reg52.h>
#include <intrins.h>
#define uchar unsigned char
#define uint unsigned int

#define LCD_data P0	//数据口
sbit LCD_RS = P3 ^ 5;  //寄存器选择输入
sbit LCD_RW = P3 ^ 6;  //液晶读/写控制
sbit LCD_EN = P3 ^ 4;  //液晶使能控制
sbit LCD_PSB = P3 ^ 7; //串/并方式控制

sbit DQ = P1 ^ 0;   //ds18b20信号位
sbit Data = P1 ^ 1; //定义dh11数据线

sbit key1 = P3 ^ 4;
sbit key2 = P3 ^ 5;
sbit key3 = P3 ^ 6;
sbit key4 = P3 ^ 7;

#define delayNOP() \
	;              \
	{              \
		_nop_();   \
		_nop_();   \
		_nop_();   \
		_nop_();   \
	};

uchar row, column, num;
uchar STcurrent;
uchar ST[] = "ST:  .  R:none ";
uchar AT[] = "AT:  .  R:24-25";
uchar SH[] = "SH:  .  R:45-50";
uchar AH[] = "AH:  .  R:20-30";

void delay(int ms)
{
	while (ms--)
	{
		uchar i;
		for (i = 0; i < 250; i++)
		{
			_nop_();
			_nop_();
			_nop_();
			_nop_();
		}
	}
}

/*******************************************************************/
/*                                                                 */
/*检查LCD忙状态                                                    */
/*lcd_busy为1时，忙，等待。lcd-busy为0时,闲，可写指令与数据。      */
/*                                                                 */
/*******************************************************************/
bit lcd_busy()
{
	bit result;
	LCD_RS = 0;
	LCD_RW = 1;
	LCD_EN = 1;
	delayNOP();
	result = (bit)(P0 & 0x80);
	LCD_EN = 0;
	return (result);
}

/*******************************************************************/
/*                                                                 */
/*写指令数据到LCD                                                  */
/*RS=L，RW=L，E=高脉冲，D0-D7=指令码。                             */
/*                                                                 */
/*******************************************************************/
void lcd_wcmd(uchar cmd)
{
	while (lcd_busy())
		;
	LCD_RS = 0;
	LCD_RW = 0;
	LCD_EN = 0;
	_nop_();
	_nop_();
	P0 = cmd;
	delayNOP();
	LCD_EN = 1;
	delayNOP();
	LCD_EN = 0;
}

/*******************************************************************/
/*                                                                 */
/*写显示数据到LCD                                                  */
/*RS=H，RW=L，E=高脉冲，D0-D7=数据。                               */
/*                                                                 */
/*******************************************************************/
void lcd_wdat(uchar dat)
{
	while (lcd_busy())
		;
	LCD_RS = 1;
	LCD_RW = 0;
	LCD_EN = 0;
	P0 = dat;
	delayNOP();
	LCD_EN = 1;
	delayNOP();
	LCD_EN = 0;
}

void lcd_init()
{
	LCD_PSB = 1;	//并口方式
	lcd_wcmd(0x31); //扩展指令操作
	delay(5);
	lcd_wcmd(0x01);
	delay(5);
	LCD_RS = 0;
	LCD_RW = 0;
	lcd_wcmd(0x0c);
	delay(5);

	/*lcd_wcmd(0x0f); //显示开，关光标
	delay(5);
	lcd_wcmd(0x01);		   //清除LCD的显示内容
	delay(5);*/
}

/*********************************************************/
/*                                                       */
/* 设定显示位置                                          */
/*                                                       */
/*********************************************************/
void lcd_pos(uchar X, uchar Y)
{
	uchar pos;
	if (X == 0)
	{
		X = 0x80;
	}
	else if (X == 1)
	{
		X = 0x90;
	}
	else if (X == 2)
	{
		X = 0x88;
	}
	else if (X == 3)
	{
		X = 0x98;
	}
	pos = X + Y;
	lcd_wcmd(pos); //显示地址
}

void printcursor()
// 打印光标函数
{
	lcd_wcmd(0x34);
	delay(2);
	if (row == 0 || row == 1)
	{
		lcd_wcmd(0x80 + 14 + 16 * row);
		delay(2);
		lcd_wcmd(0x80 + column / 2);
		delay(2);
		if (column % 2 == 0)
		{
			lcd_wdat(0x7f);
			delay(2);
			lcd_wdat(0x00);
		}
		else
		{
			lcd_wdat(0x00);
			delay(2);
			lcd_wdat(0x7f);
		}
	}
	else if (row == 2 || row == 3)
	{
		lcd_wcmd(0x80 + 14 + 16 * (row - 2));
		delay(2);
		lcd_wcmd(0x88 + column / 2);
		delay(2);
		if (column % 2 == 0)
		{
			lcd_wdat(0x7f);
			delay(2);
			lcd_wdat(0x00);
		}
		else
		{
			lcd_wdat(0x00);
			delay(2);
			lcd_wdat(0x7f);
			delay(2);
		}
	}
	lcd_wcmd(0x36);
}

void cleanpastcursor(int i, int j)
// 光标移动后，将上一个光标显示清除掉
{
	lcd_wcmd(0x34);
	delay(2);
	if (i == 0 || i == 1)
	{
		lcd_wcmd(0x80 + 14 + 16 * i);
		delay(2);
		lcd_wcmd(0x80 + j / 2);
		delay(2);
		lcd_wdat(0x00);
		delay(2);
		lcd_wdat(0x00);
	}
	else if (i == 2 || i == 3)
	{
		lcd_wcmd(0x80 + 14 + 16 * (i - 2));
		delay(2);
		lcd_wcmd(0x88 + j / 2);
		delay(2);
		lcd_wdat(0x00);
		delay(2);
		lcd_wdat(0x00);
		delay(2);
	}
	lcd_wcmd(0x36);
}

void delayms(int x)
{
	int i, j;
	for (i = 0; i < x; i++)
		for (j = 0; j < 110; j++)
			;
}

void delay_18b20(unsigned int i)
{
	while (i--)
		;
}

void Init_DS18B20(void)
{
	unsigned char x = 0;
	DQ = 1;			 // DQ复位
	delay_18b20(4);  //稍做延时
	DQ = 0;			 //单片机将DQ拉低
	delay_18b20(80); //精确延时 大于 480us
	DQ = 1;			 //拉高总线
	delay_18b20(10);
	x = DQ; //稍做延时后 如果x=0则初始化成功 x=1则初始化失败
	delay_18b20(20);
}

unsigned char ReadOneChar(void)
{
	unsigned char i = 0;
	unsigned char dat = 0;
	for (i = 8; i > 0; i--)
	{
		DQ = 0; // 给脉冲信号
		dat >>= 1;
		DQ = 1; // 给脉冲信号
		if (DQ)
			dat |= 0x80;
		delay_18b20(4);
	}
	return (dat);
}

void WriteOneChar(unsigned char dat)
{
	unsigned char i = 0;
	for (i = 8; i > 0; i--)
	{
		DQ = 0;
		DQ = dat & 0x01;
		delay_18b20(5);
		DQ = 1;
		dat >>= 1;
	}
}

unsigned int ReadTemperature(void)
{
	unsigned char a, b;
	unsigned int temp;
	float t;
	Init_DS18B20();
	WriteOneChar(0xCC); // 跳过读序号列号的操作
	WriteOneChar(0x44); // 启动温度转换
	Init_DS18B20();
	WriteOneChar(0xCC); //跳过读序号列号的操作
	WriteOneChar(0xBE); //读取温度寄存器等（共可读9个寄存器） 前两个就是温度
	a = ReadOneChar();
	b = ReadOneChar();
	temp = b;
	temp <<= 8;
	temp = temp | a;
	t = temp * 0.625;
	temp = (int)t;
	return (temp);
}

void loadSTcurrent()
/* 空气温度：ds18b20返回一个上百的数字，把这个数字转换成字符 */
{
	ST[3] = STcurrent / 100 + 48;
	ST[4] = (STcurrent / 10) % 10 + 48;
	ST[6] = STcurrent % 10 + 48;
}

void DHT11_delay_us(uchar n)
{
	while (--n)
		;
}

void DHT11_delay_ms(uint z)
{
	uint i, j;
	for (i = z; i > 0; i--)
		for (j = 110; j > 0; j--)
			;
}

void DHT11_start()
{
	Data = 1;
	DHT11_delay_us(2);
	Data = 0;
	DHT11_delay_ms(20); //延时18ms以上
	Data = 1;
	DHT11_delay_us(30);
}

uchar DHT11_rec_byte() //接收一个字节
{
	uchar i, dat = 0;
	for (i = 0; i < 8; i++) //从高到低依次接收8位数据
	{
		while (!Data)
			;			   ////等待50us低电平过去
		DHT11_delay_us(8); //延时60us，如果还为高则数据为1，否则为0
		dat <<= 1;		   //移位使正确接收8位数据，数据为0时直接移位
		if (Data == 1)	 //数据为1时，使dat加1来接收数据1
			dat += 1;
		while (Data)
			; //等待数据线拉低
	}
	return dat;
}

void DHT11_receive() //接收40位的数据
{
	uchar R_H, R_L, T_H, T_L, RH, RL, TH, TL, revise;
	DHT11_start();
	if (Data == 0)
	{
		while (Data == 0)
			;					   //等待拉高
		DHT11_delay_us(40);		   //拉高后延时80us
		R_H = DHT11_rec_byte();	//接收湿度高八位
		R_L = DHT11_rec_byte();	//接收湿度低八位
		T_H = DHT11_rec_byte();	//接收温度高八位
		T_L = DHT11_rec_byte();	//接收温度低八位
		revise = DHT11_rec_byte(); //接收校正位

		DHT11_delay_us(25); //结束

		if ((R_H + R_L + T_H + T_L) == revise) //校正
		{
			RH = R_H;
			RL = R_L;
			TH = T_H;
			TL = T_L;
		}
		/*数据处理，方便显示*/
		AH[3] = '0' + (RH / 10);
		AH[4] = '0' + (RH % 10);
		AH[6] = '0' + (int)(RL * 0.0039 * 10.0); //显示小数位
		AT[3] = '0' + (TH / 10);
		AT[4] = '0' + (TH % 10);
		AT[6] = '0' + (int)(TL * 0.0039 * 10.0); //显示小数位
	}
}

void keyscan() //按键扫描函数
{
	if (key1 == 0)
	{
		delayms(10);
		if (key1 == 0)
		{
			num++;
			if (num == 99)
				num = 0;
			while (!key1)
				;
		}
	}

	if (key2 == 0)
	{
		delayms(10);
		if (key2 == 0)
		{
			num--;
			if (num == 99)
				num = 0;
			while (!key2)
				;
		}
	}

	if (key3 == 0)
	{
		delayms(10);
		if (key3 == 0)
		{
			num = 0;
			while (!key3)
				;
		}
	}

	if (key4 == 0)
	{
		delayms(10);
		if (key4 == 0)
		{
			TR0 = ~TR0;
			while (!key4)
				;
		}
	}
}

int main()
{
	uchar k;
	delay(10); //延时函数

	lcd_init();		//初始化LCD
	Init_DS18B20(); //初始化DS18B20

	while (1)
	{
		STcurrent = ReadTemperature();
		loadSTcurrent();

		DHT11_receive();

		lcd_pos(0, 0); //设置显示位置为第1行的第1个字符
		for (k = 0; k < 15; k++)
		{
			lcd_wdat(ST[k]);
		}

		lcd_pos(1, 0); //设置显示位置为第2行的第1个字符
		for (k = 0; k < 15; k++)
		{
			lcd_wdat(SH[k]);
		}

		lcd_pos(2, 0); //设置显示位置为第3行的第1个字符
		for (k = 0; k < 15; k++)
		{
			lcd_wdat(AT[k]);
		}

		lcd_pos(3, 0); //设置显示位置为第4行的第1个字符
		for (k = 0; k < 15; k++)
		{
			lcd_wdat(AH[k]);
		}
		delayms(1500); //延时1.5秒
	}
}