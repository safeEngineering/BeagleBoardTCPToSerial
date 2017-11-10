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

#include "bbb.h"

unsigned char pwm_is_enabled = 0;

/**
 * @brief Given the header and pin number return the gpio group number
 * @param header Either P8 or P9
 * @param pin Pin number on the header (not the gpio number)
 * @return The gpio group number
 */
int gpio_group(int header, int pin)
{
    int i;
    int p8_gpio1[] = {3, 4, 5, 6, 11,12,15,16,20,21, 22,23,24,25,26,12};
    int p8_gpio2[] = {18,27,28,29,30,39,40,41,42,43, 44,45,46};
    int p9_gpio1[] = {12,15,23};
    int p9_gpio3[] = {25,27};

    if (header == P8) {
        for (i = 0; i < 16; i++)
            if (pin == p8_gpio1[i]) return 1;
        for (i = 0; i < 13; i++)
            if (pin == p8_gpio2[i]) return 2;
    }
    else {
        for (i = 0; i < 3; i++)
            if (pin == p9_gpio1[i]) return 1;
        for (i = 0; i < 2; i++)
            if (pin == p9_gpio3[i]) return 3;
    }

    return 0;
}

/**
 * @brief Given the header and pin number return the gpio number
 * @param header Either P8 or P9
 * @param pin Pin number on the header (not the gpio number)
 * @return The gpio number
 */
int header_pin(int header, int pin)
{
    int i;
    int p8_gpio1[] = {3, 4, 5, 6, 11,12,15,16,20,21, 22,23,24,25,26,12};
    int p8_gpio1_pin[] = {P8_3, P8_4, P8_5, P8_6, P8_11,P8_12,P8_15,P8_16,P8_20,P8_21, P8_22,P8_23,P8_24,P8_25,P8_26,P8_12};
    int p8_gpio2[] = {18,27,28,29,30,39,40,41,42,43, 44,45,46};
    int p8_gpio2_pin[] = {P8_18,P8_27,P8_28,P8_29,P8_30,P8_39,P8_40,P8_41,P8_42,P8_43, P8_44,P8_45,P8_46};
    int p9_gpio1[] = {12,15,23};
    int p9_gpio1_pin[] = {P9_12,P9_15,P9_23};
    int p9_gpio3[] = {25,27};
    int p9_gpio3_pin[] = {P9_25,P9_27};

    if (header == P8) {
        for (i = 0; i < 16; i++)
            if (pin == p8_gpio1[i]) return p8_gpio1_pin[i];
        for (i = 0; i < 13; i++)
            if (pin == p8_gpio2[i]) return p8_gpio2_pin[i];
    }
    else {
        for (i = 0; i < 3; i++)
            if (pin == p9_gpio1[i]) return p9_gpio1_pin[i];
        for (i = 0; i < 2; i++)
            if (pin == p9_gpio3[i]) return p9_gpio3_pin[i];
    }

    return 0;
}

/**
 * @brief Sets an integer value for a sysfs entry
 * @param path The sysfs path for the value
 * @param value The value to be set
 * @return Zero on success
 */
int set_sysfs_value(char * path, int value)
{
   FILE * fp;
   int pwmchip_number;
   char sysfs_path[512];

   for (pwmchip_number = 0; pwmchip_number < 10; pwmchip_number++) {
       sprintf(sysfs_path, path, pwmchip_number);
       fp = fopen(sysfs_path, "w");
       if (!fp) continue;
       fprintf(fp, "%d", value);
       fclose(fp);
       break;
   }
   if (pwmchip_number == 10) return 1;
   return 0;
}

/**
 * @brief Sets a string value for a sysfs entry
 * @param path The sysfs path for the value
 * @param value The value to be set
 * @return Zero on success
 */
/*
int set_sysfs_string(char * path, char * value)
{
   FILE * fp;
   fp = fopen(path, "w");
   if (!fp) return 1;
   fprintf(fp, "%s", value);
   fclose(fp);
   return 0;
}
*/

/**
 * @brief Reads an analog input
 * @param index Index of the input
 * @param value The returned value
 * @return zero on success
 */
int analog_input(int index, int * value)
{
    FILE * fp;
    int value_exists=0;
    char filename[256],valuestr[256];
    sprintf(filename, ANALOG_VALUE, index);
    fp = fopen(filename, "rt");
    if (!fp) return 1;

    while (!feof(fp)) {
        if (fgets(valuestr , 255 , fp) != NULL) {
            *value = atoi(valuestr);
            value_exists=1;
        }
    }
    fclose(fp);

    if (value_exists == 0) return 2;
    return 0;
}

volatile void * get_gpio_addr(int fd, int header, int pin)
{
    switch(gpio_group(header, pin)) {
    case 0: {
        return mmap(0, GPIO0_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO0_START_ADDR);
    }
    case 1: {
        return mmap(0, GPIO1_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO1_START_ADDR);
    }
    case 2: {
        return mmap(0, GPIO2_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO2_START_ADDR);
    }
    case 3: {
        return mmap(0, GPIO3_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO3_START_ADDR);
    }
    }
    return MAP_FAILED;
}

/**
 * @brief Sets a digital output
 * @param header P8 or P9
 * @param pin Pin number on the header
 * @param value The value to set
 * @return zero on success
 */
int digital_output(int header, int pin, int value)
{
    struct gpio_reg gpio1 = { NULL, NULL, NULL};
    volatile void *gpio_addr = NULL;
    unsigned int tmp_reg;

    int fd = open("/dev/mem", O_RDWR);
    gpio_addr = get_gpio_addr(fd, header, pin);

    if (gpio_addr == MAP_FAILED) {
        printf("Unable to map GPIO\n");
        return 1;
    }

    gpio1.oe_addr = (void*)((char*)gpio_addr + GPIO_OE);
    gpio1.setdataout_addr = (void*)((char*)gpio_addr + GPIO_SETDATAOUT);
    gpio1.cleardataout_addr = (void*)((char*)gpio_addr + GPIO_CLEARDATAOUT);

    tmp_reg = *gpio1.oe_addr;
    tmp_reg &= ~(1<<header_pin(header,pin));
    *gpio1.oe_addr = tmp_reg;
    if (value != 0)
        *gpio1.setdataout_addr = (1<<header_pin(header, pin));
    else
        *gpio1.cleardataout_addr = (1<<header_pin(header, pin));
    close(fd);
    return 0;
}

int enable_pwm()
{
    FILE * fp;
    //int i;

    fp = fopen("/tmp/enable_pwm", "w");
    if (!fp) return 1;

    fprintf(fp, "config-pin -l %s.%d\n", "P9", 14);
    fprintf(fp, "config-pin %s.%d pwm\n", "P9", 14);
    fprintf(fp, "config-pin -l %s.%d\n", "P9", 16);
    fprintf(fp, "config-pin %s.%d pwm\n", "P9", 16);
    fprintf(fp, "config-pin -l %s.%d\n", "P8", 13);
    fprintf(fp, "config-pin %s.%d pwm\n", "P8", 13);
    fprintf(fp, "config-pin -l %s.%d\n", "P8", 19);
    fprintf(fp, "config-pin %s.%d pwm\n", "P8", 19);

    fprintf(fp, "for i in `find /sys |grep pwmchip|grep /export$`\n");
    fprintf(fp, "do\n");
    fprintf(fp, "    if [ -f $i ]; then\n");
    fprintf(fp, "        echo $1 > $i\n");
    fprintf(fp, "    fi\n");
    fprintf(fp, "done\n\n");

    fprintf(fp, "for i in `find /sys |grep pwmchip|grep /pwm$1 |grep enable$`\n");
    fprintf(fp, "do\n");
    fprintf(fp, "    echo 1 > $i\n");
    fprintf(fp, "done\n\n");

    fprintf(fp, "for i in `find /sys |grep pwmchip|grep /pwm$1 |grep period$`\n");
    fprintf(fp, "do\n");
    fprintf(fp, "    echo $2 > $i\n");
    fprintf(fp, "done\n\n");

    fprintf(fp, "for i in `find /sys |grep pwmchip|grep /pwm$1 |grep duty_cycle$`\n");
    fprintf(fp, "do\n");
    fprintf(fp, "    echo $3 > $i\n");
    fprintf(fp, "done");
    fclose(fp);

    system("bash /tmp/enable_pwm 0 2000 0");
    system("bash /tmp/enable_pwm 1 2000 0");

    pwm_is_enabled = 1;
    return 0;
}

/**
 * @brief Sets a pwm output
 * @param header P8 or P9
 * @param pin Pin number on the header
 * @param value The value to set in the range 0-1000
 * @return zero on success
 */
int pwm_output(int header, int pin, int value)
{
    int pwm_pin[] = {P9, 14,
                     P9, 16,
                     P8, 13,
                     P8, 19};
    int i, on_off=1;
    char headerstr[3]; //cmdstr[256];

    for (i = 0; i < 4; i++) {
        if ((pwm_pin[i*2] == header) && (pwm_pin[i*2+1] == pin)) {
            break;
        }
    }

    /* pin not found */
    if (i == 4) return 1;

    if (header==P8)
        sprintf(headerstr, "%s", "P8");
    else
        sprintf(headerstr, "%s", "P9");

    if (pwm_is_enabled == 0)
        enable_pwm();

    if (value == 0) on_off = 0;

    switch(i) {
    case 0: { /* P9_14 */
        if (set_sysfs_value("/sys/devices/platform/ocp/48302000.epwmss/48302200.pwm/pwm/pwmchip%d/pwm0/enable", on_off) != 0) return 2;
        if (set_sysfs_value("/sys/devices/platform/ocp/48302000.epwmss/48302200.pwm/pwm/pwmchip%d/pwm0/period", 2000) != 0) return 3;
        if (set_sysfs_value("/sys/devices/platform/ocp/48302000.epwmss/48302200.pwm/pwm/pwmchip%d/pwm0/duty_cycle", value*2) != 0) return 4;
        break;
    }
    case 1: { /* P9_16 */
        if (set_sysfs_value("/sys/devices/platform/ocp/48302000.epwmss/48302200.pwm/pwm/pwmchip%d/pwm1/enable", on_off) != 0) return 5;
        if (set_sysfs_value("/sys/devices/platform/ocp/48302000.epwmss/48302200.pwm/pwm/pwmchip%d/pwm1/period", 2000) != 0) return 6;
        if (set_sysfs_value("/sys/devices/platform/ocp/48302000.epwmss/48302200.pwm/pwm/pwmchip%d/pwm1/duty_cycle", value*2) != 0) return 7;
        break;
    }
    case 2: { /* P8_13 */
        if (set_sysfs_value("/sys/devices/platform/ocp/48304000.epwmss/48304200.pwm/pwm/pwmchip%d/pwm1/enable", on_off) != 0) return 8;
        if (set_sysfs_value("/sys/devices/platform/ocp/48302000.epwmss/48302200.pwm/pwm/pwmchip%d/pwm1/period", 2000) != 0) return 9;
        if (set_sysfs_value("/sys/devices/platform/ocp/48304000.epwmss/48304200.pwm/pwm/pwmchip%d/pwm1/duty_cycle", value*2) != 0) return 10;
        break;
    }
    case 3: { /* P8_19 */
        if (set_sysfs_value("/sys/devices/platform/ocp/48304000.epwmss/48304200.pwm/pwm/pwmchip%d/pwm0/enable", on_off) != 0) return 11;
        if (set_sysfs_value("/sys/devices/platform/ocp/48302000.epwmss/48302200.pwm/pwm/pwmchip%d/pwm0/period", 2000) != 0) return 12;
        if (set_sysfs_value("/sys/devices/platform/ocp/48304000.epwmss/48304200.pwm/pwm/pwmchip%d/pwm0/duty_cycle", value*2) != 0) return 13;
        break;
    }
    }

    return 0;
}

/**
 * @brief Given the header and pin number return the gpio number
 *        as it appears in /sys/class/gpio
 * @param header P8 or P9
 * @param pin Pin number (not the gpio number)
 * @return gpio number
 */
int gpio_index_from_header_pin(int header, int pin)
{
    int i;

    int pin_mapping[] = {
        P8, 3, 38,
        P8, 4, 39,
        P8, 5, 34,
        P8, 6, 35,
        P8, 11, 45,
        P8, 12, 44,
        P8, 15, 47,
        P8, 16, 46,
        P8, 20, 63,
        P8, 21, 62,

        P8, 22, 37,
        P8, 23, 36,
        P8, 24, 33,
        P8, 25, 32,
        P8, 26, 61,
        P9, 12, 60,
        P9, 15, 48,
        P9, 23, 49,
        P8, 18, 65,
        P8, 27, 86,

        P8, 28, 88,
        P8, 29, 87,
        P8, 30, 89,
        P8, 39, 76,
        P8, 40, 77,
        P8, 41, 74,
        P8, 42, 75,
        P8, 43, 72,
        P8, 44, 73,
        P8, 45, 70,
        P8, 46, 71,

        P9, 25, 117,
        P9, 27, 115
    };

    for (i = 0; i < 33; i++)
        if ((header == pin_mapping[i*3]) &&
            (pin == pin_mapping[i*3+1]))
            return pin_mapping[i*3+2];
    return 0;
}

/**
 * @brief Reads a digital input from the given header pin
 * @param header P8 or P9
 * @param pin Pin number (not the gpio number)
 * @return The state of the input
 */
int digital_input(int header, int pin)
{
    FILE * fp;
    int index, value=-1, value_exists=0;
    char filename[256],valuestr[256];

    index = gpio_index_from_header_pin(header, pin);
    if (index == 0) return -1;
    sprintf(filename, GPIO_VALUE, index);
    fp = fopen(filename, "rt");
    if (!fp) return -2;

    while (!feof(fp)) {
        if (fgets(valuestr , 255 , fp) != NULL) {
            value = atoi(valuestr);
            value_exists=1;
        }
    }
    fclose(fp);

    if (value_exists == 0) return -3;
    return value;
}

/**
 * @brief Returns zero if analog inputs are enabled
 * @returns Zero if analog inputs are available
 */
int analog_inputs_available()
{
    FILE * fp;
    int enabled=0;
    char filename[256],linestr[256];
    sprintf(filename, BONE_SLOTS);
    fp = fopen(filename, "rt");
    if (!fp) return -1;

    while (!feof(fp)) {
        if (fgets(linestr , 255 , fp) != NULL) {
            if (strstr(linestr,"BB-ADC") != NULL) enabled=1;
        }
    }
    fclose(fp);
    if (enabled == 0) return -2;
    return 0;
}

/**
 * @brief Enables analog inputs within sysfs
 */
int enable_analog_inputs()
{
    char cmdstr[256];

    if (analog_inputs_available() != 0) {
        sprintf(cmdstr, "echo BB-ADC > %s", BONE_SLOTS);
        printf("%s\n", cmdstr);
        system(cmdstr);
        sleep(2);
    }

    if (analog_inputs_available() != 0)
        return 1;

    return 0;
}

/**
 * @brief returns a character representing high or low input states
 * @param header P8 or P9
 * @param pin Pin number on the header
 * @return Character representing pin state
 */
char show_digital_input_char(int header, int pin)
{
    int state = digital_input(header, pin);
    if (state == 1) return 'O';
    return '.';
}

/**
 * @brief Shows an ascii diagram of the headers for debugging purposes
 */
void show_gpio()
{
    int i;

    printf("         P9                    P8\n\n");
    for (i = 0; i < 46/2; i++) {
    printf("      %02d %c%c %02d              %02d %c%c %02d\n",
               i*2+1, show_digital_input_char(P9,i*2+1), show_digital_input_char(P9,i*2+2), i*2+2,
               i*2+1, show_digital_input_char(P8,i*2+1), show_digital_input_char(P8,i*2+2), i*2+2);
    }
}

/**
 * @brief Set direction of digital pin
 * @param header P8 or P9
 * @param pin Pin number on the header
 * @param dir Direction of pin (IN oe OUT)
 */
int digital_direction(int header, int pin, int dir)
{
    FILE * fp;
    int index;
    char filename[256];

    index = gpio_index_from_header_pin(header, pin);
    if (index == 0) return -1;
    sprintf(filename, GPIO_DIR, index);
    fp = fopen(filename, "w");
    if (!fp)
    {
        fp = fopen("/sys/class/gpio/export", "w");
        if (!fp) return -1;
        fprintf(fp, "%d", index);
        fclose(fp);
        sleep(1);
        // Retry to open IO file again
        fp = fopen(filename, "w");
        if (!fp) return -1;        
    }
    
    if(dir == IN)
        fprintf(fp, "%s", "in");
    else
        fprintf(fp, "%s", "out");
    fclose(fp);
    
    return 0;
}