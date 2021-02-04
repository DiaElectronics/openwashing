/*
 * buttons.c
 *
 *  Created on: Jan 28, 2021
 *      Author: Roman Fedyashov
 */


#include "buttons.h"

void add_button(buttons * obj, uint16_t pin, GPIO_TypeDef * port) {
	obj->btn[obj->cursor].pin = pin;
	obj->btn[obj->cursor].port = port;
	obj->btn[obj->cursor].digital_state = 1; //1 means released, no macros deliberately
	obj->btn[obj->cursor].is_clicked = 0;
	obj->btn[obj->cursor].bounce_analog_state = BOUNCE_SAMPLES; // maximum released state
	obj->cursor = obj->cursor + 1;
}

void init_buttons(buttons * obj) {
	obj->cursor = 0;
	add_button(obj, BUTTON_1_Pin, BUTTON_1_GPIO_Port);
	add_button(obj, BUTTON_2_Pin, BUTTON_2_GPIO_Port);
	add_button(obj, BUTTON_3_Pin, BUTTON_3_GPIO_Port);
}

void update_button(button * obj) {
  if (HAL_GPIO_ReadPin (obj->port, obj->pin)) {
	  // positive value is when a button released
	  obj->bounce_analog_state++;
	  if (obj->bounce_analog_state >= BOUNCE_SAMPLES) {
		  // really released the button
		  obj->bounce_analog_state = BOUNCE_SAMPLES;
		  if(!obj->digital_state) {
			  obj->digital_state = 1;
			  // let's consider button as completely released now
			  // only completely released button can be clicked again
		  }
	  }
  } else {
	  // negative value means the key was actually pressed
	  obj->bounce_analog_state--;
	  if (obj->bounce_analog_state<=0) {
		  // really pressed the button
		  obj->bounce_analog_state = 0;
		  if(obj->digital_state) {
			  // if was released before
			  obj->digital_state = 0;
			  obj->is_clicked = 1;
		  }
	  }
  }
}
