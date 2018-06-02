/*
 * @Author: Guoquan Wei 1940359148@qq.com 
 * @Date: 2018-05-03 21:03:48 
 * @Last Modified by: Guoquan Wei
 * @Last Modified time: 2018-05-17 13:51:40
 */

#include <reg52.h>
#include <intrins.h>
#define uchar unsigned char
#define uint unsigned int

#define LCD_data P0	//数据口
sbit LCD_PSB = P3 ^ 3; //串/并方式控制
sbit LCD_EN = P3 ^ 4;  //液晶使能控制
sbit LCD_RS = P3 ^ 5;  //寄存器选择输入
sbit LCD_RW = P3 ^ 6;  //液晶读/写控制

sbit Buzzer = P2 ^ 0; //蜂鸣器
sbit Data = P2 ^ 1;   //定义dh11数据线at and ah
sbit SHdata = P2 ^ 2; //土壤湿度模块
sbit DQ = P2 ^ 3;	 //ds18b20信号位st

sbit key1 = P1 ^ 4;
sbit key2 = P1 ^ 5;
sbit key3 = P1 ^ 6;
sbit key4 = P1 ^ 7;

sbit Heater = P1 ^ 0;
sbit Air_blower = P1 ^ 1;
sbit Sprayer = P1 ^ 2;
sbit Pump = P1 ^ 3;

#define delayNOP() \
	;              \
	{              \
		_nop_();   \
		_nop_();   \
		_nop_();   \
		_nop_();   \
	};

uchar num = 0, num1 = 0, num2 = 0, num3 = 0; //num,num2,num3分别是按键1,2,3按下后改变的参数，用这些参数的变化通过keyload函数使按键显示在lcd上,num1是中断函数的参数
uchar flag = 0;
uchar Buzzerflag = 1;
uchar SHmin = 20, SHmax = 80, ATmin = 20, ATmax = 40, AHmin = 10, AHmax = 70;
unsigned int STcurrent;
uchar AHcurrent, ATcurrent, SHcurrent;
uchar ST[] = "ST:  . ";
uchar SH[] = "SH:  . ";
uchar AT[] = "AT:  . ";
uchar AH[] = "AH:  . ";
uchar STrange[] = "R: none ";
uchar SHrange[] = "R:20--80";
uchar ATrange[] = "R:20--40";
uchar AHrange[] = "R:10--70";

void trans_num_to_char(uchar a, uchar *s)
/* 将数字如SHmin转行到字符数字SH中的对应位置 */
{
	*s = a / 10 + 48;
	*(s + 1) = a % 10 + 48;
}

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

void printrange()
{
	uchar k;
	lcd_pos(0, 4); //设置显示位置为第1行的第1个字符
	for (k = 0; k < 8; k++)
	{
		lcd_wdat(STrange[k]);
	}
	lcd_pos(1, 4); //设置显示位置为第2行的第1个字符
	for (k = 0; k < 8; k++)
	{
		lcd_wdat(SHrange[k]);
	}
	lcd_pos(2, 4); //设置显示位置为第3行的第1个字符
	for (k = 0; k < 8; k++)
	{
		lcd_wdat(ATrange[k]);
	}
	lcd_pos(3, 4); //设置显示位置为第4行的第1个字符
	for (k = 0; k < 8; k++)
	{
		lcd_wdat(AHrange[k]);
	}
}

void lcd_init()
{
	LCD_RS = 0;
	LCD_RW = 0;
	LCD_PSB = 1;	//并口方式
	lcd_wcmd(0x31); //扩展指令操作
	delay(5);
	lcd_wcmd(0x01); //清除显示
	delay(5);
	lcd_wcmd(0x0f);
	//显示开，开光标
	delay(5);
	printrange();
	/*lcd_wcmd(0x0f); //显示开，开光标
	delay(5);
	lcd_wcmd(0x01);		   //清除LCD的显示内容
	delay(5);*/
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

void loadcurrent(unsigned int a, uchar *p)
/* 空气温度：ds18b20返回一个上百的数字，把这个数字转换成字符 */
{
	*(p + 3) = a / 100 + 48;
	if (*(p + 3) == '0')
	{
		*(p + 3) = ' ';
	}
	*(p + 4) = (a / 10) % 10 + 48;
	*(p + 6) = a % 10 + 48;
}

void loadcurrent_two(unsigned char a, uchar *p)
{
	*(p + 3) = (a / 10) + 48;
	if (*(p + 3) == '0')
	{
		*(p + 3) = ' ';
	}
	*(p + 4) = a % 10 + 48;
	*(p + 6) = '0';
}

void DHT11_delay_us(uchar n)
{
	while (--n)
		;
}

void DHT11_start()
{
	Data = 1;
	DHT11_delay_us(2);
	Data = 0;
	delayms(20); //延时18ms以上
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
	uchar R_H, R_L, T_H, T_L, RL, TL, revise;
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
			AHcurrent = R_H;
			RL = R_L;
			ATcurrent = T_H;
			TL = T_L;
		}
		/*数据处理，方便显示*/
		AH[3] = '0' + (AHcurrent / 10);
		AH[4] = '0' + (AHcurrent % 10);
		AH[6] = '0' + (int)(RL * 0.0039 * 10.0); //显示小数位
		AT[3] = '0' + (ATcurrent / 10);
		AT[4] = '0' + (ATcurrent % 10);
		AT[6] = '0' + (int)(TL * 0.0039 * 10.0); //显示小数位
	}
}

void keyscan() //按键扫描函数
{
	key1 = key2 = key3 = key4 = 1;
	if (key1 == 0)
	/* key1按下后，每个num对应不同的光标位置 */
	{
		delayms(10);
		if (key1 == 0)
		{
			num++;
			if (num == 7)
			{
				num = 0;
			}
			flag = 1;
			while (!key1)
				;
		}
	}

	if (key2 == 0)
	{
		delayms(10);
		if (key2 == 0)
		{
			if (num != 0)
			{
				num2 = 1;
			}
			while (!key2)
				;
		}
	}

	if (key3 == 0)
	{
		delayms(10);
		if (key3 == 0)
		{
			if (num != 0)
			{
				num3 = 1;
			}
			while (!key3)
				;
		}
	}
}

void keyload()
{
	if (num == 0)
	{
		lcd_wcmd(0x0c);
	}
	else if (num == 1)
	{
		lcd_wcmd(0x0f);
		if (flag == 1)
		{
			lcd_pos(1, 5);
			flag = 0;
		}
		if (num2)
		{
			if (SHmax - SHmin > 1)
			{
				SHmin++;
			}
			num2 = 0;
			trans_num_to_char(SHmin, SHrange + 2);
			printrange();
			lcd_pos(1, 5);
		}
		if (num3)
		{
			if (SHmin > 10)
			{
				SHmin--;
			}
			num3 = 0;
			trans_num_to_char(SHmin, SHrange + 2);
			printrange();
			lcd_pos(1, 5);
		}
	}
	else if (num == 2)
	{
		lcd_wcmd(0x0f);
		if (flag == 1)
		{
			lcd_pos(1, 7);
			flag = 0;
		}
		if (num2)
		{
			if (SHmax < 99)
			{
				SHmax++;
			}
			num2 = 0;
			trans_num_to_char(SHmax, SHrange + 6);
			printrange();
			lcd_pos(1, 7);
		}
		if (num3)
		{
			if (SHmax - SHmin > 1)
			{
				SHmax--;
			}
			num3 = 0;
			trans_num_to_char(SHmax, SHrange + 6);
			printrange();
			lcd_pos(1, 7);
		}
	}
	else if (num == 3)
	{
		lcd_wcmd(0x0f);
		if (flag == 1)
		{
			lcd_pos(2, 5);
			flag = 0;
		}
		if (num2)
		{
			if (ATmax - ATmin > 1)
			{
				ATmin++;
			}
			num2 = 0;
			trans_num_to_char(ATmin, ATrange + 2);
			printrange();
			lcd_pos(2, 5);
		}
		if (num3)
		{
			if (ATmin > 0)
			{
				ATmin--;
			}
			num3 = 0;
			trans_num_to_char(ATmin, ATrange + 2);
			printrange();
			lcd_pos(2, 5);
		}
	}
	else if (num == 4)
	{
		lcd_wcmd(0x0f);
		if (flag == 1)
		{
			lcd_pos(2, 7);
			flag = 0;
		}
		if (num2)
		{
			if (ATmax < 99)
			{
				ATmax++;
			}
			num2 = 0;
			trans_num_to_char(ATmax, ATrange + 6);
			printrange();
			lcd_pos(2, 7);
		}
		if (num3)
		{
			if (ATmax - ATmin > 1)
			{
				ATmax--;
			}
			num3 = 0;
			trans_num_to_char(ATmax, ATrange + 6);
			printrange();
			lcd_pos(2, 7);
		}
	}
	else if (num == 5)
	{
		lcd_wcmd(0x0f);
		if (flag == 1)
		{
			lcd_pos(3, 5);
			flag = 0;
		}
		if (num2)
		{
			if (AHmax - AHmin > 1)
			{
				AHmin++;
			}
			num2 = 0;
			trans_num_to_char(AHmin, AHrange + 2);
			printrange();
			lcd_pos(3, 5);
		}
		if (num3)
		{
			if (AHmin > 0)
			{
				AHmin--;
			}
			num3 = 0;
			trans_num_to_char(AHmin, AHrange + 2);
			printrange();
			lcd_pos(3, 5);
		}
	}
	else if (num == 6)
	{
		lcd_wcmd(0x0f);
		if (flag == 1)
		{
			lcd_pos(3, 7);
			flag = 0;
		}
		if (num2)
		{
			if (AHmax < 99)
			{
				AHmax++;
			}
			num2 = 0;
			trans_num_to_char(AHmax, AHrange + 6);
			printrange();
			lcd_pos(3, 7);
		}
		if (num3)
		{
			if (AHmax - AHmin > 1)
			{
				AHmax--;
			}
			num3 = 0;
			trans_num_to_char(AHmax, AHrange + 6);
			printrange();
			lcd_pos(3, 7);
		}
	}
}

void compare()
{
	if (SHcurrent <= SHmin)
	{
		Pump = 0;
		Buzzerflag = 0;
	}
	else
	{
		Pump = 1;
	}

	if (ATcurrent <= ATmin)
	{
		Heater = 0;
		Buzzerflag = 0;
	}
	else
	{
		Heater = 1;
	}

	if (AHcurrent <= AHmin)
	{
		Sprayer = 0;
		Buzzerflag = 0;
	}
	else
	{
		Sprayer = 1;
	}

	if (ATcurrent >= ATmax || AHcurrent >= AHmax)
	{
		Air_blower = 0;
		Buzzerflag = 0;
	}
	else
	{
		Air_blower = 1;
	}

	if (SHcurrent > SHmin && ATcurrent > ATmin && ATcurrent < ATmax && AHcurrent > AHmin && AHcurrent < AHmax)
	{
		Buzzer = 1;
		Buzzerflag = 1;
	}
}

void receive_SH()
{
	bit tmp;
	uchar i, k;
	for (i = 0; i < 8; i++)
	{
		tmp = SHdata;
		if (tmp == 1)
		{
			k++;
		}
		delayms(10);
	}
	switch (k)
	{
	case 0:
		SHcurrent = (SHmax + SHmin) / 2;
		break;
	case 1:
		SHcurrent = (SHmax + SHmin) / 2;
		break;
	case 2:
		SHcurrent = SHmin - 10;
		break;
	case 3:
		SHcurrent = SHmin - 10;
		break;
	case 4:
		SHcurrent = SHmin - 10;
		break;
	case 5:
		SHcurrent = SHmin - 10;
		break;
	case 6:
		SHcurrent = SHmin - 10;
		break;
	case 7:
		SHcurrent = SHmin - 10;
		break;
	default:
		SHcurrent = SHmin - 10;
		break;
	}
}

void init_timer()
{
	TMOD = 0x01;
	TH0 = (65536 - 45872) / 256;
	TL0 = (65536 - 45872) % 256;
	EA = 1;
	ET0 = 1;
	TR0 = 1;
}

int main()
{
	uchar k;
	delay(10); //延时函数

	init_timer();
	lcd_init();		//初始化LCD
	Init_DS18B20(); //初始化DS18B20
	while (1)
	{
		keyscan();
		keyload();

		if (num == 0)
		/* num=0时，温室处于工作状态，num不等于0时，处于设定range状态，温室不工作 */
		{
			STcurrent = ReadTemperature();
			loadcurrent(STcurrent, ST);

			receive_SH();
			loadcurrent_two(SHcurrent, SH);
			/* 读取温度，并把st转成字符数字，加载近去 */
			DHT11_receive();
			compare();

			lcd_pos(0, 0); //设置显示位置为第1行的第1个字符
			for (k = 0; k < 7; k++)
			{
				lcd_wdat(ST[k]);
			}
			lcd_pos(1, 0); //设置显示位置为第2行的第1个字符
			for (k = 0; k < 7; k++)
			{
				lcd_wdat(SH[k]);
			}
			lcd_pos(2, 0); //设置显示位置为第3行的第1个字符
			for (k = 0; k < 7; k++)
			{
				lcd_wdat(AT[k]);
			}
			lcd_pos(3, 0); //设置显示位置为第4行的第1个字符
			for (k = 0; k < 7; k++)
			{
				lcd_wdat(AH[k]);
			}
			lcd_pos(3, 8); //将光标移动到显示区域外，防止乱码出现
			delay(10);
		}
	}
}

void t0_time() interrupt 1

{
	TH0 = (65536 - 45872) / 256;
	TL0 = (65536 - 45872) % 256;
	if (Buzzerflag == 0)
	{
		num1++;
		if (num1 == 21)
		{
			num1 = 0;
		}
		if (num1 == 10)
		{
			Buzzer = ~Buzzer;
		}
	}
}