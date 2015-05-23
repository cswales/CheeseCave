//  How to access GPIO registers from C-code on the Raspberry-Pi
//  Example program
//  15-January-2012
//  Dom and Gert
//  Example extended to continually output to
//  YAML files for parsing by other programs
//  brianb Oct 2013


// Access from ARM Running Linux


#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

// want time n such?
// #define __USE_POSIX 1

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <bcm2835.h>

#define MAXTIMINGS 100

//#define DEBUG 1

#define DHT11 11
#define DHT22 22
#define AM2302 22

// hard code the sensor type, can turn it back to a parameter if you like
#define SENSOR_TYPE DHT22

void log_err(char *fmt, ... ) {

  // get current time - "canonical" (not-exactly-iso8601?)
  time_t now_time = time(NULL);
  struct tm now_tm = {0};
  localtime_r(&now_time, &now_tm);
  char now_str[40];
  strftime(now_str, sizeof now_str, "%FT%T%Z ", &now_tm);
  fputs(now_str,stderr);

	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fflush(stderr);
}

void log_debug(char *fmt, ... ) {
  // get current time - "canonical" (not-exactly-iso8601?)
  time_t now_time = time(NULL);
  struct tm now_tm = {0};
  localtime_r(&now_time, &now_tm);
  char now_str[40];
  strftime(now_str, sizeof now_str, "%FT%T%Z ", &now_tm);
  fputs(now_str,stdout);

	va_list args;
	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);
	fflush(stdout);
}


// read only globals set from command line
//
int     g_pin;
char    *g_sensor_name;
char    *g_dir_name;
char    g_delay;
char    g_history_fn[255];
char    g_stats_fn[255];


int sample_sensor(void) {

  int counter = 0;
  int laststate = HIGH;
  int j=0;

#ifdef DEBUG
  log_debug( "sampling sensor %s at pin %d\n",g_sensor_name,g_pin);
#endif

  // Set GPIO pin to output
  bcm2835_gpio_fsel(g_pin, BCM2835_GPIO_FSEL_OUTP);

  bcm2835_gpio_write(g_pin, HIGH);
  usleep(500000);
  bcm2835_gpio_write(g_pin, LOW);
  usleep(20000);

  // Set GPIO to input
  bcm2835_gpio_fsel(g_pin, BCM2835_GPIO_FSEL_INPT);


  // wait for pin to drop?
  counter = 0;
  while (bcm2835_gpio_lev(g_pin) == 1) {
    usleep(1);
    if (counter++ > 100000) {
	   log_err( "pin never dropped\n");
	   return(-1);
    }
  }

  int bits[250], data[100];

  data[0] = data[1] = data[2] = data[3] = data[4] = 0;
  int bitidx = 0;

  // read data!
  for (int i=0; i< MAXTIMINGS; i++) {
    counter = 0;
    while ( bcm2835_gpio_lev(g_pin) == laststate) {
      counter++;
      //nanosleep(1);   // overclocking might change this?
      if (counter == 1000)
        break;
    }
    laststate = bcm2835_gpio_lev(g_pin);
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
    log_debug("bit %d: %d\n", i-3, bits[i]);
    log_debug("bit %d: %d (%d)\n", i-2, bits[i+1], bits[i+1] > 200);
  }
#endif

  printf("Data (%d): 0x%x 0x%x 0x%x 0x%x 0x%x\n", j, data[0], data[1], data[2], data[3], data[4]);

  if ((j < 39) ||
      (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
    //
    log_err( "Poor man's checksum failed.\n");
    return(-1);
  }

  float c = 0.0, f=0.0, h=0.0;

  // yay!
  if (SENSOR_TYPE == DHT11) {
    printf("Temp = %d *C, Hum = %d \%\n", data[2], data[0]);
    c = (float) data[2];
    h = (float) data[0];
  }

  if (SENSOR_TYPE == DHT22) {

    h = data[0] * 256 + data[1];
    h /= 10;

    c = (data[2] & 0x7F)* 256 + data[3];
    c /= 10.0;
    if (data[2] & 0x80)  c *= -1;

  }

  f = ((c * 9.0) / 5.0) + 32.0;
  printf("Temp =  %.1f *C %.1f *F, Hum = %.1f \%\n", c, f, h);

  // some basic data validation
  if (f < 1.0) {
	log_err( "do not believe temp is less than 1 degree F\n");
	return(-1);
  }
  if (f > 150.0) {
    log_err( "do not believe temp is greater than 150 degree F\n");
    return(-1);
  }
  if (h < 1.0) {
    log_err( "do not believe humidity is less than 1\n");
	return(-1);
  }
  if (h > 101.0) {
    log_err( "do not believe humidity is greater than 100\n");
    return(-1);
  }

  // get current time - "canonical" (not-exactly-iso8601?)
  time_t now_time = time(NULL);
  struct tm now_tm = {0};
  localtime_r(&now_time, &now_tm);
  char now_str[40];
  strftime(now_str, sizeof now_str, "%FT%T%Z", &now_tm);

  // update the history.json file
  // format is "sequence of maps" which looks like:
  // - { sensor: sname, time: XXXX, epoch: YYYYY, temperature: ZZ.Z, humidity: LL.L }

  FILE *fp = fopen(g_history_fn, "a");
  if (fp == NULL) return(-1);

// This is hard to parse, it turns out. a "list of maps" means you have to keep the whole thing in memory.
//  fprintf(fp, "- { sensor: %s,time: \"%s\",epoch: %d, temperature: %.1f, celsius: %.1f, humidity: %.1f }\n",g_sensor_name,now_str,now_time,f,c,h);

// This is better YAML (according to YAML 1.2) because it creates a stream of documents.
//  fprintf(fp, "--- { sensor: %s,time: \"%s\",epoch: %d, temperature: %.1f, celsius: %.1f, humidity: %.1f }\n",g_sensor_name,now_str,now_time,f,c,h);

// But let's face it, the world is JSON now.
// If you separate your JSON objects by carriage return, you'll get a single file that's not a "real .JSON" 
// but it's " streaming JSON " that most parsers will happily deal with - without blowing up memory.
// Let's use that as the default now.
	fprintf(fp, "{ sensor: %s,time: \"%s\",epoch: %d, temperature: %.1f, celsius: %.1f, humidity: %.1f }\n",g_sensor_name,now_str,now_time,f,c,h);

  fclose(fp);

  // Also update the stats.yaml - and do the safe thing with writing to a temp then swapping the file
  // format: just sensor: sname\ntime: XXX .... as above

  char tmpName[256];
  tmpnam(tmpName);
  fp = fopen(tmpName,"w");
  if (fp == NULL) {
    fprintf(stderr, "could not open temp file for output");
    return(-1);
  }

  fprintf(fp, "sensor: %s\n",g_sensor_name);
  fprintf(fp, "time: %s\n",now_str);
  fprintf(fp, "epoch: %d\n",now_time);
  fprintf(fp, "temperature: %.1f\n",f);
  fprintf(fp, "humidity: %.1f\n",h);
  fprintf(fp, "celsius: %.1f\n",c);

  fclose(fp);

  // mv to correct location
  if (0 != rename(tmpName,g_stats_fn)) {
    fprintf(stderr, "could not move to output file");
    return(-1);
  }

  return 0;
}

int validate_pin (int pin) {
  switch (pin) {
    case 2:
    case 3:
    case 4:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 14:
    case 15:
    case 16:
    case 18:
    case 22:
    case 23:
    case 24:
    case 27:
      break;

    default:
      return(-1);
  }
  return(0); // valid
}

void usage(void) {
  fprintf(stderr, "usage:\n");
  fprintf(stderr, "  out-dir sensor-name pin-number secs-delay \n");

  fprintf(stderr, "  the file 'sensor-name-history.json' will be updated,\n");
  fprintf(stderr, "  and sensor-name-stats.yaml with just the current,\n");
  fprintf(stderr, "  and it happens every secs-delay seconds (although if we get a GPIO error\n");
  fprintf(stderr, "  we just skip a slot (assumes DHT22 but you can change #define\n");
}

int main(int argc, char **argv)
{


  if (argc != 5) {
    usage();
    exit(-1);
  }

  char *out_dir = argv[1];
  g_sensor_name = argv[2];
  g_pin = atoi(argv[3]);
  g_delay = atoi(argv[4]);

  log_debug("Starting err_log_DHT: out_dir %s sensor_name %s pin %d delay %d\n",out_dir,g_sensor_name,g_pin,g_delay);
  log_err("Starting err_log_DHT: out_dir %s sensor_name %s pin %d delay %d\n",out_dir,g_sensor_name,g_pin,g_delay);

  if (0 != validate_pin(g_pin)) {
    log_err(" pin %d is not valid for the raspberry pi\n",g_pin);
    return(-1);
  }

  if (!bcm2835_init()) {
	log_err("Could not init the bcm2835 chip, should never happen\n");
    return 1;
  }

  snprintf(g_history_fn,sizeof(g_history_fn),"%s/%s-history.json",out_dir,g_sensor_name);
  snprintf(g_stats_fn,sizeof(g_stats_fn),"%s/%s-stats.yaml",out_dir,g_sensor_name);

  do {

    sample_sensor();

    usleep( g_delay * 1000 * 1000);

  } while (1);

  return(0);

} // main


