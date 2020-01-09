#include "usart.h"
#include "delay.h"
#pragma import(__use_no_semihosting)			 
//标准库需要的支持函数				 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;	   
//定义_sys_exit()以避免使用半主机模式	
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART3->SR&0X40)==0);//循环发送,直到发送完毕   
	USART3->DR = (u8) ch;	  
	return ch;
}

UART_HandleTypeDef usart3_handler; 	
u8 USART_RX_BUF[USART_REC_LEN] __attribute__ ((at(0X20001000)));//接收缓冲,最大USART_REC_LEN个字节,起始地址为0X20001000.  

u16 USART_RX_STA=0;	   //接收状态标记	
u32 USART_RX_CNT=0;			//接收的字节数 
u8 file_over=0;
u8 file_start=0;
u8 aRxBuffer[RXBUFFERSIZE];//HAL库使用的串口接收缓冲
u8 flag_file = 0;
 
void uart_init(u32 bound)
{
	usart3_handler.Instance=USART3;						//USART3
	usart3_handler.Init.BaudRate=bound;					//
	usart3_handler.Init.WordLength=UART_WORDLENGTH_8B;   
	usart3_handler.Init.StopBits=UART_STOPBITS_1;		
	usart3_handler.Init.Parity=UART_PARITY_NONE;			
	usart3_handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   
	usart3_handler.Init.Mode=UART_MODE_TX_RX;		   
	HAL_UART_Init(&usart3_handler);						
}
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
   
	GPIO_InitTypeDef GPIO_Initure;
	
	if(huart->Instance==USART3)
	{
		__HAL_RCC_GPIOB_CLK_ENABLE();			
		__HAL_RCC_USART3_CLK_ENABLE();		
	
		GPIO_Initure.Pin=GPIO_PIN_10;			//PB10
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;	
		GPIO_Initure.Pull=GPIO_PULLUP;		
		GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;		
		GPIO_Initure.Alternate=GPIO_AF7_USART3;//????
		HAL_GPIO_Init(GPIOB,&GPIO_Initure);	   	

		GPIO_Initure.Pin=GPIO_PIN_11;			//PB11
		HAL_GPIO_Init(GPIOB,&GPIO_Initure);	   	
		__HAL_UART_DISABLE_IT(huart,UART_IT_TC);
		
		__HAL_UART_ENABLE_IT(huart,UART_IT_RXNE);
		HAL_NVIC_EnableIRQ(USART3_IRQn);	
		HAL_NVIC_SetPriority(USART3_IRQn,3,3);	
	}
	
}

void USART3_IRQHandler(void)   //
{
	u8 Res;
	if((__HAL_UART_GET_FLAG(&usart3_handler,UART_FLAG_RXNE)!=RESET))  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
	HAL_UART_Receive(&usart3_handler,&Res,1,1000); 
		if(USART_RX_CNT<USART_REC_LEN)
		{
			if(Res == '#')
			{
				USART_RX_BUF[USART_RX_CNT]=Res;
				file_over++;
				if(file_over==10)
				{
					flag_file =1;
					HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_0);
					for(int i=0;i<10;i++)
					{
						USART_RX_BUF[USART_RX_CNT]=0x00;
						USART_RX_CNT--;
						file_over--;
					}
				}
				USART_RX_CNT++;
			}
			else if(Res=='&'){
				USART_RX_BUF[USART_RX_CNT]=Res;
				file_start++;
				if(file_start==10)
				{
					int tmp = USART_RX_CNT;
					USART_RX_CNT=0;
					for(int i=tmp;i>0;i--)
					{
						USART_RX_BUF[i]=0x00;
					}
					file_start=0;
					return;
				}
				USART_RX_CNT++;
			}
			else{
				file_over=0;
				file_start=0;
				USART_RX_BUF[USART_RX_CNT]=Res;
				USART_RX_CNT++;
			}				
		}			 
	}
	HAL_UART_IRQHandler(&usart3_handler);
	
}
	

