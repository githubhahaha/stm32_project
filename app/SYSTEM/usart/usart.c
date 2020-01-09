#include "usart.h"
#include "timer.h"
#include "delay.h"
#pragma import(__use_no_semihosting)			 
//��׼����Ҫ��֧�ֺ���				 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;	   
//����_sys_exit()�Ա���ʹ�ð�����ģʽ	
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
	while((USART3->SR&0X40)==0);//ѭ������,ֱ���������   
	USART3->DR = (u8) ch;	  
	return ch;
}

UART_HandleTypeDef usart3_handler; 	
UART_HandleTypeDef usart2_handler; 
u8 USART_RX_BUF[USART_REC_LEN] __attribute__ ((at(0X20001000)));//���ջ���,���USART_REC_LEN���ֽ�,��ʼ��ַΪ0X20001000.  

//GPS 
u16 USART2_RX_STA=0; 
//���ڷ��ͻ����� 	
__align(8) u8 USART2_TX_BUF[USART2_MAX_SEND_LEN]; 	//���ͻ���,���USART3_MAX_SEND_LEN�ֽ�  	  
//���ڽ��ջ����� 	
u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 				//���ջ���,���USART3_MAX_RECV_LEN���ֽ�.


u16 USART_RX_STA=0;	   //����״̬���	
u32 USART_RX_CNT=0;			//���յ��ֽ��� 

u8 aRxBuffer[RXBUFFERSIZE];//HAL��ʹ�õĴ��ڽ��ջ���

u8 com_flag=0;//��ָͨ��
 
void uart_init(u32 bound)//��NBͨ��
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
void uart2_init(u32 bound)//��GPSͨ��
{	
	//UART ��ʼ������
	usart2_handler.Instance=USART2;					    //USART2
	usart2_handler.Init.BaudRate=bound;				    //������
	usart2_handler.Init.WordLength=UART_WORDLENGTH_8B;   //�ֳ�Ϊ8λ���ݸ�ʽ
	usart2_handler.Init.StopBits=UART_STOPBITS_1;	    //һ��ֹͣλ
	usart2_handler.Init.Parity=UART_PARITY_NONE;		    //����żУ��λ
	usart2_handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //��Ӳ������
	usart2_handler.Init.Mode=UART_MODE_TX_RX;		    //�շ�ģʽ
	HAL_UART_Init(&usart2_handler);					    //HAL_UART_Init()��ʹ��UART1
}
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
   
	if(huart->Instance==USART3)
	{
		GPIO_InitTypeDef GPIO_Initure1;
		__HAL_RCC_GPIOB_CLK_ENABLE();			
		__HAL_RCC_USART3_CLK_ENABLE();		
	
		GPIO_Initure1.Pin=GPIO_PIN_10;			//PB10
		GPIO_Initure1.Mode=GPIO_MODE_AF_PP;	
		GPIO_Initure1.Pull=GPIO_PULLUP;		
		GPIO_Initure1.Speed=GPIO_SPEED_FREQ_HIGH;		
		GPIO_Initure1.Alternate=GPIO_AF7_USART3;//????
		HAL_GPIO_Init(GPIOB,&GPIO_Initure1);	   	

		GPIO_Initure1.Pin=GPIO_PIN_11;			//PB11
		HAL_GPIO_Init(GPIOB,&GPIO_Initure1);	   	
		__HAL_UART_DISABLE_IT(huart,UART_IT_TC);
		
		__HAL_UART_ENABLE_IT(huart,UART_IT_RXNE);
		HAL_NVIC_EnableIRQ(USART3_IRQn);	
		HAL_NVIC_SetPriority(USART3_IRQn,3,3);	
	}
	if(huart->Instance==USART2)
	{
		  //GPIO�˿�����
		GPIO_InitTypeDef GPIO_Initure;
	
		__HAL_RCC_GPIOA_CLK_ENABLE();			//ʹ��GPIOAʱ��
		__HAL_RCC_USART2_CLK_ENABLE();			//ʹ��USART2ʱ��
	
		GPIO_Initure.Pin=GPIO_PIN_2;			//PA2
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//�����������
		GPIO_Initure.Pull=GPIO_PULLUP;			//����
		GPIO_Initure.Speed=GPIO_SPEED_FAST;		//����
		GPIO_Initure.Alternate=GPIO_AF7_USART2;	//����ΪUSART2
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//��ʼ��PA2

		GPIO_Initure.Pin=GPIO_PIN_3;			//PA3
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//��ʼ��PA3

		__HAL_UART_ENABLE_IT(huart,UART_IT_RXNE);		//���������ж�
		HAL_NVIC_EnableIRQ(USART2_IRQn);				//ʹ��USART2�ж�
		HAL_NVIC_SetPriority(USART2_IRQn,2,3);			//��ռ���ȼ�2�������ȼ�3	
		TIM7_Int_Init(1000-1,9000-1);		//100ms�ж�
		USART2_RX_STA=0;		//����
		TIM7->CR1&=~(1<<0);        //�رն�ʱ��7
	}
}

void USART3_IRQHandler(void)   //��NBͨ��
{
	u8 Res;
	if((__HAL_UART_GET_FLAG(&usart3_handler,UART_FLAG_RXNE)!=RESET))  //�����ж�
	{
	  HAL_UART_Receive(&usart3_handler,&Res,1,1000); 
		if(USART_RX_CNT<USART_REC_LEN)
		{
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_1);
			if(Res=='!')
			{
				com_flag=1;
			}
			else
			{
				USART_RX_BUF[USART_RX_CNT]=Res;
				USART_RX_CNT++;
			}
				
		}
	}
	HAL_UART_IRQHandler(&usart3_handler);
}
void USART2_IRQHandler(void)
{
	u8 res;	      
	if(__HAL_UART_GET_FLAG(&usart2_handler,UART_FLAG_RXNE)!=RESET)//���յ�����
	{	 
		res=USART2->DR; 			 
		if((USART2_RX_STA&(1<<15))==0)//�������һ������,��û�б�����,���ٽ�����������
		{ 
			if(USART2_RX_STA<USART2_MAX_RECV_LEN)	//�����Խ�������
			{
				TIM7->CNT=0;         				//���������	
				if(USART2_RX_STA==0) 				//ʹ�ܶ�ʱ��7���ж� 
				{
					TIM7->CR1|=1<<0;     			//ʹ�ܶ�ʱ��7
				}
				USART2_RX_BUF[USART2_RX_STA++]=res;	//��¼���յ���ֵ	 
			}else 
			{
				USART2_RX_STA|=1<<15;				//ǿ�Ʊ�ǽ������
			} 
		}
	}  				 											 
}   
	

