/*
 * gpio_app.h
 *
 *  Created on: Jan 28, 2021
 *      Author: Roman Fedyashov
 */

#ifndef INC_GPIO_APP_H_
#define INC_GPIO_APP_H_
#include "main.h"

#define GPIO_KEYS_SUPPORTED 11

typedef struct relay_t {
  uint16_t pin;
  GPIO_TypeDef * port;
} relay;

typedef struct gpio_entity_s {
	int cursor;
	relay relays[GPIO_KEYS_SUPPORTED];
} gpio_entity;

void add_relay(gpio_entity *obj, uint16_t pin, GPIO_TypeDef * port);
void init_gpio_entity(gpio_entity *obj);

#endif /* INC_GPIO_APP_H_ */
