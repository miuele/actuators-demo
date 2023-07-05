/* mbed Microcontroller Library
 * Copyright (c) 2023 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"

#include "ezmot/ezmot.h"
#include "eaux/eaux.h"

constexpr float k_duty_step = 0.02f;
constexpr float k_duty_max = 0.5f;
constexpr float k_duty_safe_to_rev = 0.1f;

using eaux::clamp;
using ezmot::EasyMotor;

UnbufferedSerial g_serial(CONSOLE_TX, CONSOLE_RX);

DigitalOut g_air(PA_4);
EasyMotor<PwmOut> g_mot(PA_1, PA_3, 50us);

char serial_read_char(UnbufferedSerial &ser) {
	char c;
	ser.read(&c, 1);
	return c;
}

int main()
{
	printf("#ready\n");

	bool rev = false;
	float duty = 0.f;
	while (true) {
		float duty_user = std::abs(duty);

		char c = serial_read_char(g_serial);

		switch (c) {
			case 'k':
				duty_user += k_duty_step;
				break;

			case 'j':
				duty_user -= k_duty_step;
				break;

			case ' ':
				duty_user = 0.f;
				break;

			case 'r':
				if (duty_user > k_duty_safe_to_rev) {
					printf("#motor too fast to revert\n");
				} else {
					rev = !rev;
					printf("#rev: %d\n", rev);
				}
				break;

			case 't':
			{
				int state = g_air.read();
				g_air.write(!state);
				printf("#air now set to %d\n", !state);
				break;
			}

			default:
				printf("#bad input: ignored\n");
				break;
		}

		if (c == 'k' || c == 'j' || c == ' ') {
			duty = (rev ? -1 : 1) * clamp(duty_user, 0.f, k_duty_max);

			g_mot.set_duty(duty);

			printf("#duty now set to %f(%c)\n", std::abs(duty), (rev ? '-' : '+'));
		}
	}
}
