#include "key.h"

#define INTERVAL 10

void init(void);
void tick_event(void);

key_t wkup, key1, key0;
uint8_t read_key0() {return GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5);};
uint8_t read_key1() {return GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15);};
uint8_t read_wkup() {return GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0);};
void wkup_down_event(void* key);
void key1_down_event(void* key);
void key0_down_event(void* key);
void wkup_long_free_event(void* key);
void key1_long_event(void* key);
void key0_long_event(void* key);
void wkup_double_event(void* key);
void key1_double_event(void* key);
void key0_double_event(void* key);


int main(void)
{
	init();
	 
	while(1)
	{
		tick_event();
        Delay_ms(INTERVAL);
	}
}
 

void init()
{
	/* LED ��ʼ�� */
	LED_GPIO_Config();
	
	/* ���ڳ�ʼ�� */
	USART_Config();
  
	/* ������ʼ�� */
    Key_GPIO_Config();

  
    key_create("WKUP", &wkup, read_wkup, 1);
    key_create("KEY1", &key1, read_key1, 0);
	key_create("KEY0", &key0, read_key0, 0);
    key_connect(&wkup, KEY_DOWN, wkup_down_event);
    key_connect(&key1, KEY_DOWN, key1_down_event);
	key_connect(&key0, KEY_DOWN, key0_down_event);
    key_connect(&wkup, KEY_LONG_FREE, wkup_long_free_event);
    key_connect(&key1, KEY_LONG, key1_long_event);
	key_connect(&key0, KEY_LONG, key0_long_event);
    key_connect(&wkup, KEY_DOUBLE, wkup_double_event);
    key_connect(&key1, KEY_DOUBLE, key1_double_event);
    key_connect(&key0, KEY_DOUBLE, key0_double_event);
}


void tick_event()
{
	key_process();
};

void wkup_down_event(void* key)
{
	PRINT_INFO("wkup ����!");

}

void key1_down_event(void* key)
{
	PRINT_INFO("key1 ����!");

}

void key0_down_event(void* key)
{
	PRINT_INFO("key0 ����!");
}

void wkup_long_free_event(void* key)
{
	PRINT_INFO("wkup �����ͷ�!");
}

void key1_long_event(void* key)
{
	PRINT_INFO("key1 ����!");
}

void key0_long_event(void* key)
{
	PRINT_INFO("key0 ����!");
}

void wkup_double_event(void* key)
{
	PRINT_INFO("wkup ˫��!");
}

void key1_double_event(void* key)
{
	PRINT_INFO("key1 ˫��!");
}

void key0_double_event(void* key)
{
	PRINT_INFO("key0 ˫��!");
}
