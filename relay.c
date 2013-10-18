//  How to access GPIO registers from C-code on the Raspberry-Pi
//  Example program
//  15-January-2012
//  Dom and Gert
//


// Access from ARM Running Linux


#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <bcm2835.h>

#define MAXTIMINGS 100

//#define DEBUG
/*
#define DHT11 11
#define DHT22 22
#define AM2302 22

int bits[250], data[100];
int bitidx = 0;

int readDHT(int type, int pin) {
  int counter = 0;
  int laststate = HIGH;
  int j=0;

  // Set GPIO pin to output
  bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);

  bcm2835_gpio_write(pin, HIGH);
  usleep(500000);
  bcm2835_gpio_write(pin, LOW);
  usleep(20000);

  // Set GPIO to input
  bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);

  data[0] = data[1] = data[2] = data[3] = data[4] = 0;

  // wait for pin to drop?
  while (bcm2835_gpio_lev(pin) == 1) {
    usleep(1);
  }

  // read data!
  for (int i=0; i< MAXTIMINGS; i++) {
    counter = 0;
    while ( bcm2835_gpio_lev(pin) == laststate) {
      counter++;
      //nanosleep(1);   // overclocking might change this?
      if (counter == 1000)
        break;
    }
    laststate = bcm2835_gpio_lev(pin);
    if (counter == 1000) break;
    bits[bitidx++] = counter;

    if ((i>3) && (i%2 == 0)) {
      // shove each bit into the storage bytes
      data[j/8] <<= 1;
      if (counter > 200) {
        data[j/8] |= 1;
      }
      j++;
    }
  }


#ifdef DEBUG
  for (int i=3; i<bitidx; i+=2) {
    printf("bit %d: %d\n", i-3, bits[i]);
    printf("bit %d: %d (%d)\n", i-2, bits[i+1], bits[i+1] > 200);
  }
#endif

  printf("Data (%d): 0x%x 0x%x 0x%x 0x%x 0x%x\n", j, data[0], data[1], data[2], data[3], data[4]);

  if ((j >= 39) &&
      (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {

    // yay!
    if (type == DHT11) {
      printf("Temp = %d *C, Hum = %d \%\n", data[2], data[0]);
    }

    if (type == DHT22) {
      float c, f, h;
      h = data[0] * 256 + data[1];
      h /= 10;

      c = (data[2] & 0x7F)* 256 + data[3];
      c /= 10.0;
      if (data[2] & 0x80)  c *= -1;
      f = ((c * 9.0) / 5.0) + 32.0;
      printf("Temp =  %.1f *C %.1f *F, Hum = %.1f \%\n", c, f, h);

    }
    return 1;
  }

  return 0;
}
*/

void printUsage(const char *name)
{
    printf("usage: %s [on|off|state] <pin>\n", name);
    printf("<pin> defaults to 17 (GPIO17) if not specified.\n");
    printf("Allowed <pin> values are 4, 17-18, 22-25, and 27\n");
}

int main(int argc, char **argv)
{
  int gpioPin = 17;
  if (!bcm2835_init()){
  	printf("broadcom GPIO chip could not be initialized, bailing\n");
    return 1;
  }

  // check for 'on' or 'off' or 'state'
  if (argc < 2 || !(!strcmp(argv[1], "on") || !strcmp(argv[1], "off") || !strcmp(argv[1], "state"))) {
    printUsage(argv[0]);
    return 2;
  }
  
  // If there's a third argument, it better be a valid number
  if (argc >=3 ){
  	gpioPin = atoi(argv[2]);
  	if (gpioPin != 4 && gpioPin != 17 && gpioPin != 18 && 
  		!(gpioPin >= 22 && gpioPin <=25) && 
  		gpioPin != 27) {
  		printUsage(argv[0]);
  		return 2;
  	}
  }

  //printf("Using pin #%d\n", gpioPin);

  if (!strcmp(argv[1], "on")) {
  	printf("Setting pin HIGH\n");
	bcm2835_gpio_fsel(gpioPin, BCM2835_GPIO_FSEL_OUTP);
  	bcm2835_gpio_write(gpioPin, HIGH);
  } else if (!strcmp(argv[1], "off")) {
	printf("Setting pin LOW\n");
  	bcm2835_gpio_fsel(gpioPin, BCM2835_GPIO_FSEL_OUTP);
  	bcm2835_gpio_write(gpioPin, LOW);
  } else {  // state - get current state of the relay
	int state = bcm2835_gpio_lev(gpioPin);
	if (state == 0) {
		printf("off");
	} else {
		printf("on");
	}
  }	
  return 0;

} // main


