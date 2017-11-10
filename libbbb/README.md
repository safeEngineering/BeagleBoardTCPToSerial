# libbbb

C/C++ library functions for handling the I/O of the Beaglebone Black.

<img src="https://github.com/bashrc/libbbb/blob/master/img/headers.png?raw=true" width=800/>

## Installation

``` bash
git clone https://github.com/bashrc/libbbb
cd libbbb
make
sudo make install
```

## Examples

You can compile any of the following examples with the command:

``` bash
gcc -Wall -std=gnu99 -pedantic -O3 -o example *.c -I/usr/local/include/libbbb -lbbb
```

And of course to get access to the I/O run the resulting programs as root or with *sudo*.

### Set an output

Sets pin 12 on header P9 (GPIO 60) to high then low state.

``` C
#include "bbb.h"

int main(int argc, char *argv[])
{
    digital_output(P9, 12, 1);
    sleep(1);
    digital_output(P9, 12, 0);
    return 0;
}
```

### Set a Pulse Width Modulated output

Four pins can be used as Pulse Width Modulated (PWM) outputs: P9/14, P9/16, P8/19 and P8/13. These can be used to control the intensity of LEDs, the speed of motors or the position of servos. Their values can be in the range 0-1000.

``` C
#include "bbb.h"

int main(int argc, char *argv[])
{
    pwm_output(P8, 13, 1000);
    sleep(2);
    pwm_output(P8, 13, 100);
    return 0;
}
```

### Get the state of an input

Gets the state of pin 12 on header P9 (GPIO 60).

``` C
#include "bbb.h"

int main(int argc, char *argv[])
{
    printf("Pin state: %d\n", digital_input(P9, 12));
    return 0;
}
```

### Get the state of an analog input

Gets the state of analog input 0.

``` C
#include "bbb.h"

int main(int argc, char *argv[])
{
    int value=0;
    if (enable_analog_inputs() != 0) return 1;
    if (analog_input(0, &value) == 0)
        printf("Analog input 0: %d\n", value);
    return 0;
}
```

### Show the current state of all inputs

Shows an ASCII diagram with the state of digital inputs on both headers.

``` C
#include "bbb.h"

int main(int argc, char *argv[])
{
    show_gpio();
    return 0;
}
```
