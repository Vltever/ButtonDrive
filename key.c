#include "key.h"


void key_init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC,ENABLE);//使能PORTA,PORTC时钟

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭jtag，使能SWD，可以用SWD模式调试
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_15;//PA15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA15
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;//PC5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOC5
 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;//PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0设置成输入，默认下拉	  
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.0
}

static struct KEY* head_key = 0;


static char *strn_copy(char *dst, const char *src, uint32_t n);
static void __add_key(key_t* key);


void key_create(const char *name,
                  key_t *key, 
                  uint8_t(*read_key_level)(void),
                  uint8_t key_trigger_level)
{
  memset(key, 0, sizeof(struct KEY));
 
  strn_copy(key->name, name, KEY_NAME_MAX);
  
  
  key->key_state = NONE_TRIGGER;
  key->key_last_state = NONE_TRIGGER;
  key->key_trigger_event = NONE_TRIGGER;
  key->read_key_level = read_key_level;
  key->key_trigger_level = key_trigger_level;
  key->key_last_level = key->read_key_level();
  key->debounce_time = 0;
    
  __add_key(key); 
}


void key_connect(key_t *key,KEY_EVENT key_event,KEY_CALLBACK key_callback)
{
  if(KEY_ALL_TRIGGER == key_event)
  {
    for(int i = 0 ; i < number_of_event-1 ; i++)
      key->callBack_function[i] = key_callback;
  }
  else
  {
    key->callBack_function[key_event] = key_callback;
  }
}


void key_delete(key_t *key)
{
  struct KEY** curr;
  for(curr = &head_key; *curr;) 
  {
    struct KEY* entry = *curr;
    if (entry == key) 
    {
      *curr = entry->Next;
    } 
    else
    {
      curr = &entry->Next;
    }
  }
}


uint8_t get_key_event(key_t *key)
{
  return (uint8_t)(key->key_trigger_event);
}


uint8_t get_key_state(key_t *key)
{
  return (uint8_t)(key->key_state);
}


void __key_cycle_process(key_t *key)
{
  uint8_t current_level = (uint8_t)key->read_key_level();
  
  if((current_level != key->key_last_level) && (++(key->debounce_time) >= KEY_DEBOUNCE_TIME))   //电平状态改变
  {
      key->key_last_level = current_level;
      key->debounce_time = 0;
      
      if((key->key_state == NONE_TRIGGER) || (key->key_state == KEY_DOUBLE))
      {
        key->key_state = KEY_DOWN;
      }
      else if(key->key_state == KEY_DOWN)
      {
        key->key_state = KEY_UP;
        TRIGGER_CB(KEY_UP);
      }
  }
  
  switch(key->key_state)
  {
    case KEY_DOWN :
    {
      if(key->key_last_level == key->key_trigger_level)
      {
        #if CONTINUOS_TRIGGER

        if(++(key->key_cycle) >= KEY_CONTINUOS_CYCLE)
        {
          key->key_cycle = 0;
          key->key_trigger_event = KEY_CONTINUOS; 
          TRIGGER_CB(KEY_CONTINUOS);
        }

        #else
        
        if(key->key_last_state != KEY_LONG)
          key->key_trigger_event = KEY_DOWN;
      
        if(++(key->long_time) >= KEY_LONG_TIME)
        {
          if(++(key->key_cycle) >= KEY_LONG_CYCLE)
          {
            key->key_cycle = 0;
            key->key_trigger_event = KEY_LONG;
            key->key_last_state = KEY_LONG;
            TRIGGER_CB(KEY_LONG);
          }          
          if(key->long_time == 0xFF)
          {
            key->long_time = KEY_LONG_TIME;
          }
        }
        
        #endif
      }

      break;
    } 
    
    case KEY_UP :
    {
      if(key->key_trigger_event == KEY_DOWN)
      {
        if((key->timer_count <= KEY_DOUBLE_TIME)&&(key->key_last_state == KEY_DOUBLE))
        {
          key->key_trigger_event = KEY_DOUBLE;
          TRIGGER_CB(KEY_DOUBLE);    
          key->key_state = NONE_TRIGGER;
          key->key_last_state = NONE_TRIGGER;
        }
        else
        {
            key->timer_count=0;
            key->long_time = 0;
          
          #if (SINGLE_AND_DOUBLE_TRIGGER == 0)
            TRIGGER_CB(KEY_DOWN);
          #endif
            key->key_state = KEY_DOUBLE;
            key->key_last_state = KEY_DOUBLE;
          
        }
      }
      
      else if(key->key_trigger_event == KEY_LONG)
      {
        TRIGGER_CB(KEY_LONG_FREE);
        key->long_time = 0;
        key->key_state = NONE_TRIGGER;
        key->key_last_state = KEY_LONG_FREE;
      } 
      
      #if CONTINUOS_TRIGGER
        else if(key->key_trigger_event == KEY_CONTINUOS)
        {
          key->long_time = 0;
          TRIGGER_CB(KEY_CONTINUOS_FREE);
          key->key_state = NONE_TRIGGER;
          key->key_last_state = KEY_CONTINUOS;
        } 
      #endif
      
      break;
    }
    
    case KEY_DOUBLE :
    {
      key->timer_count++;
      if(key->timer_count>=KEY_DOUBLE_TIME)
      {
        key->key_state = NONE_TRIGGER;
        key->key_last_state = NONE_TRIGGER;
      }
      #if SINGLE_AND_DOUBLE_TRIGGER
      
        if((key->timer_count>=KEY_DOUBLE_TIME)&&(key->key_last_state != KEY_DOWN))
        {
          key->timer_count=0;
          TRIGGER_CB(KEY_DOWN);
          key->key_state = NONE_TRIGGER;
          key->key_last_state = KEY_DOWN;
        }
        
      #endif

      break;
    }

    default :
      break;
  }
  
}


void key_process(void)
{
  struct KEY* pass_btn;
  for(pass_btn = head_key; pass_btn != NULL; pass_btn = pass_btn->Next)
  {
		__key_cycle_process(pass_btn);
  }
}


static char *strn_copy(char *dst, const char *src, uint32_t n)
{
  if (n != 0)
  {
    char *d = dst;
    const char *s = src;
    do
    {
			if ((*d++ = *s++) == 0)
			{
				while (--n != 0)
					*d++ = 0;
				break;
			}
    } while (--n != 0);
  }
  return dst;
}


static void __add_key(key_t* key)
{
  key->Next = head_key;
  head_key = key;
}




