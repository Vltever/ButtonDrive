#ifndef _KEY_H
#define _KEY_H

#include "sys.h"
#include "string.h"

#define KEY_NAME_MAX  32


#define CONTINUOS_TRIGGER               0		    //连按为无需达到长按阈值的长按

#define SINGLE_AND_DOUBLE_TRIGGER       1

#define KEY_DEBOUNCE_TIME               2		    //消抖时间	(n-1)*INTERVAL
#define KEY_CONTINUOS_CYCLE             1		    //连按触发周期时间  (n-1)*INTERVAL
#define KEY_LONG_CYCLE                  11	    //长按触发周期时间  (n-1)*INTERVAL
#define KEY_DOUBLE_TIME                 11	    //双击间隔时间  (n-1)*INTERVAL
#define KEY_LONG_TIME                   31	    //持续((n-1)*INTERVAL ms)认为长按事件

#define TRIGGER_CB(event)   \
        if(key->callBack_function[event]) \
          key->callBack_function[event]((key_t*)key)

typedef void (*KEY_CALLBACK)(void*);


typedef enum {
  KEY_DOWN = 0,
  KEY_UP,
  KEY_DOUBLE,
  KEY_LONG,
  KEY_LONG_FREE,
  KEY_CONTINUOS,
  KEY_CONTINUOS_FREE,
  KEY_ALL_TRIGGER,
  number_of_event,
  NONE_TRIGGER
}KEY_EVENT;


typedef struct KEY
{
	uint8_t (*read_key_level)(void);
  
  char name[KEY_NAME_MAX];
  	
  uint8_t key_state              :   4;		//当前状态
  uint8_t key_last_state         :   4;		//上一次的状态
  uint8_t key_trigger_level      :   2;		//触发电平
  uint8_t key_last_level         :   2;		//当前电平
  
  uint8_t key_trigger_event;
  
  KEY_CALLBACK callBack_function[number_of_event];
  
	uint8_t key_cycle;
  
  uint8_t timer_count;
	uint8_t debounce_time;
  
	uint8_t long_time;
  
  struct KEY *Next;
  
}key_t;


void key_init(void);

void key_create(const char *name,
                  key_t *key, 
                  uint8_t(*read_key_level)(void),
                  uint8_t key_trigger_level);
                  
void key_connect(key_t *key,KEY_EVENT key_event,KEY_CALLBACK key_callback);   
                  
void __key_cycle_process(key_t *key);   
                  
void key_process(void);
                  
void key_delete(key_t *key);   
                  
uint8_t get_key_event(key_t *key);
uint8_t get_key_state(key_t *key);

#endif
