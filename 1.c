/*
 * @Author: Guoquan Wei 1940359148@qq.com 
 * @Date: 2018-05-03 21:03:48 
 * @Last Modified by: Guoquan Wei
 * @Last Modified time: 2018-05-03 21:43:26
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
sbit wela = P2 ^ 6;
sbit dula = P2 ^ 7;

#define delayNOP(); {_nop_();_nop_();_nop_();_nop_();};

uchar row,column;

void delay(int ms)
{
    while(ms--)
	{
      uchar i;
	  for(i=0;i<250;i++)  
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
   result = (bit)(P0&0x80);
   LCD_EN = 0;
   return(result); 
}

/*******************************************************************/
/*                                                                 */
/*写指令数据到LCD                                                  */
/*RS=L，RW=L，E=高脉冲，D0-D7=指令码。                             */
/*                                                                 */
/*******************************************************************/
void lcd_wcmd(uchar cmd)
{                          
   while(lcd_busy());
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
   while(lcd_busy());
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_EN = 0;
    P0 = dat;
    delayNOP();
    LCD_EN = 1;
    delayNOP();
    LCD_EN = 0; 
}

void initphoto()
{
	uchar i, j;
	lcd_wcmd(0x34); //写数据时,关闭图形显示
	for (i = 0; i < 32; i++)
	{
		lcd_wcmd(0x80 + i);		 //先写入水平坐标值
		lcd_wcmd(0x80);			 //写入垂直坐标值
		for (j = 0; j < 16; j++) //再写入两个8位元的数据
			lcd_wdat(0x00);
		delay(2);
	}

	for (i = 0; i < 32; i++)
	{
		lcd_wcmd(0x80 + i);		 //先写入水平坐标值
		lcd_wcmd(0x88);			 //写入垂直坐标值
		for (j = 0; j < 16; j++) //再写入两个8位元的数据
			lcd_wdat(0x00);
		delay(2);
	}
	lcd_wcmd(0x36); //写完数据,开图形显示
}

void lcd_init()
{
	LCD_PSB = 1; //并口方式
	initphoto();
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

int main()
{
	uchar i,k;
	uchar ST[]="ST:     R:none ";
	uchar AT[]="AT:     R:24-25";
	uchar SH[]="SH:     R:45-50";
	uchar AH[]="AH:     R:20-30";
	delay(10); //延时
	wela = 0;
	dula = 0;
	lcd_init(); //初始化LCD

	lcd_pos(0, 0); //设置显示位置为第四行的第1个字符
	for (k = 0; k < 15; k++)
	{
		lcd_wdat(ST[k]);
	}

	lcd_pos(1, 0); //设置显示位置为第四行的第1个字符
	for (k = 0; k < 15; k++)
	{
		lcd_wdat(SH[k]);
	}

	lcd_pos(2, 0); //设置显示位置为第四行的第1个字符
	for (k = 0; k < 15; k++)
	{
		lcd_wdat(AT[k]);
	}

	lcd_pos(3, 0); //设置显示位置为第四行的第1个字符
	for (k = 0; k < 15; k++)
	{
		lcd_wdat(AH[k]);
	}

	while (1)
	{
		;
	}
}