/*
  library functions for the Beaglebone Black
  Copyright (C) 2017 Bob Mottram
  bob@freedombone.net

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BBB_GPIO_H
#define BBB_GPIO_H

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

/* files within sysfs */

#define BONE_SLOTS   "/sys/devices/platform/bone_capemgr/slots"
#define GPIO_VALUE   "/sys/class/gpio/gpio%d/value"
#define ANALOG_VALUE "/sys/devices/platform/ocp/44e0d000.tscadc/TI-am335x-adc/iio:device0/in_voltage%d_raw"
#define GPIO_DIR     "/sys/class/gpio/gpio%d/direction"

/* gpio memory layout in three sections */

#define GPIO0_START_ADDR    0x44e07000
#define GPIO0_END_ADDR      0x44e08000
#define GPIO0_SIZE          (GPIO0_END_ADDR - GPIO0_START_ADDR)

#define GPIO1_START_ADDR    0x4804C000
#define GPIO1_END_ADDR      0x4804DFFF
#define GPIO1_SIZE          (GPIO1_END_ADDR - GPIO1_START_ADDR)

#define GPIO2_START_ADDR    0x41A4C000
#define GPIO2_END_ADDR      0x41A4D000
#define GPIO2_SIZE          (GPIO2_END_ADDR - GPIO2_START_ADDR)

#define GPIO3_START_ADDR    0x41A4E000
#define GPIO3_END_ADDR      0x41A4F000
#define GPIO3_SIZE          (GPIO3_END_ADDR - GPIO3_START_ADDR)

#define GPIO_OE             0x134
#define GPIO_SETDATAOUT     0x194
#define GPIO_CLEARDATAOUT   0x190

#define REG                 volatile unsigned int

/* GPIO 1 */

#define USR0_LED            (1<<21)
#define USR1_LED            (1<<22)
#define USR2_LED            (1<<23)
#define USR3_LED            (1<<24)

#define P8_3      6
#define P8_4      7
#define P8_5      2
#define P8_6      3
#define P8_11     13
#define P8_12     12
#define P8_15     15
#define P8_16     14
#define P8_20     31
#define P8_21     30
#define P8_22     5
#define P8_23     4
#define P8_24     1
#define P8_25     0
#define P8_26     29
#define P9_12     28
#define P9_15     16
#define P9_23     17
#define GPIO1_38  P8_3
#define GPIO1_39  P8_4
#define GPIO1_34  P8_5
#define GPIO1_35  P8_6
#define GPIO1_45  P8_11
#define GPIO1_44  P8_12
#define GPIO1_47  P8_15
#define GPIO1_46  P8_16
#define GPIO1_63  P8_20
#define GPIO1_62  P8_21
#define GPIO1_37  P8_22
#define GPIO1_36  P8_23
#define GPIO1_33  P8_24
#define GPIO1_32  P8_25
#define GPIO1_61  P8_26
#define GPIO1_60  P9_12
#define GPIO1_48  P9_15
#define GPIO1_49  P9_23

/* GPIO 2 */

#define P8_18     1
#define P8_27     22
#define P8_28     24
#define P8_29     23
#define P8_30     25
#define P8_39     12
#define P8_40     13
#define P8_41     10
#define P8_42     11
#define P8_43     8
#define P8_44     9
#define P8_45     6
#define P8_46     7
#define GPIO2_65  P8_18
#define GPIO2_86  P8_27
#define GPIO2_88  P8_28
#define GPIO2_87  P8_29
#define GPIO2_89  P8_30
#define GPIO2_76  P8_39
#define GPIO2_77  P8_40
#define GPIO2_74  P8_41
#define GPIO2_75  P8_42
#define GPIO2_72  P8_43
#define GPIO2_73  P8_44
#define GPIO2_70  P8_45
#define GPIO2_71  P8_46

/* GPIO 3 */

#define P9_25     21
#define P9_27     19
#define GPIO3_117 P9_25
#define GPIO3_115 P9_27

#define P8 0
#define P9 1

#define IN  1
#define OUT 2

typedef struct gpio_reg {
    REG *oe_addr;
    REG *setdataout_addr;
    REG *cleardataout_addr;
} gpio_reg;

int enable_analog_inputs();
int digital_input(int header, int pin);
int digital_output(int header, int pin, int value);
int pwm_output(int header, int pin, int value);
int analog_input(int index, int * value);
void show_gpio();
int digital_direction(int header, int pin, int dir);

#ifdef __cplusplus
}
#endif

#endif
