/*
 * gpio_app.c
 *
 *  Created on: Jan 28, 2021
 *      Author: roman
 */

#include "relays.h"

void add_relay(gpio_entity *obj, uint16_t pin, GPIO_TypeDef * port) {
	obj->relays[obj->cursor].pin = pin;
	obj->relays[obj->cursor].port = port;
	obj->cursor = obj->cursor + 1;
}

void init_gpio_entity(gpio_entity *obj) {
	obj->cursor = 0;
	add_relay(obj, OUT_1_Pin, OUT_1_GPIO_Port);
	add_relay(obj, OUT_2_Pin, OUT_2_GPIO_Port);
	add_relay(obj, OUT_3_Pin, OUT_3_GPIO_Port);
	add_relay(obj, OUT_4_Pin, OUT_4_GPIO_Port);
	add_relay(obj, OUT_5_Pin, OUT_5_GPIO_Port);
	add_relay(obj, OUT_6_Pin, OUT_6_GPIO_Port);
	add_relay(obj, OUT_7_Pin, OUT_7_GPIO_Port);
	add_relay(obj, OUT_8_Pin, OUT_8_GPIO_Port);
	add_relay(obj, OUT_9_Pin, OUT_9_GPIO_Port);
	add_relay(obj, OUT_10_Pin, OUT_10_GPIO_Port);
	add_relay(obj, OUT_11_Pin, OUT_11_GPIO_Port);
}
