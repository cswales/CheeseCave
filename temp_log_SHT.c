// Compile with: gcc -o testSHT1x ./../bcm2835-1.3/src/bcm2835.c ./RPi_SHT1x.c testSHT1x.c

/*
Raspberry Pi SHT1x communication library.
By:      John Burns (www.john.geek.nz)
Date:    14 August 2012
License: CC BY-SA v3.0 - http://creativecommons.org/licenses/by-sa/3.0/

This is a derivative work based on
	Name: Nice Guy SHT11 library
	By: Daesung Kim
	Date: 04/04/2011
	Source: http://www.theniceguy.net/2722

Dependencies:
	BCM2835 Raspberry Pi GPIO Library - http://www.open.com.au/mikem/bcm2835/

Sensor:
	Sensirion SHT11 Temperature and Humidity Sensor interfaced to Raspberry Pi GPIO port

*/

/*
 * modified by Brian Bulkowski, brian@bulkowski.org,
 * to output repeatedly to a YAML file that can be used to drive history
 * (and perhaps RRD real soon)
 *
 * 
*/


#include <unistd.h>
#include <errno.h>
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

#include "RPi_SHT1x.h"

#define DEBUG 1

// quick n cheap but nice little logs
// can easily be adapted to be multithread
// or a config instead of a define for outputting debug


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

#ifdef DEBUG

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

#else

void log_debug(char *fmt, ... ) {
  return;
}

#endif



// read only globals set from command line
//
int     g_clock_pin, g_data_pin;
char    *g_sensor_name;
char    *g_dir_name;
char    g_delay;
char    g_history_fn[255];
char    g_stats_fn[255];

int init_sensor(void) {

#ifdef DEBUG
	log_debug( "init SHT11 %s at data pin %d clock pin %d\n",g_sensor_name,g_data_pin,g_clock_pin);	
#endif
	
	// Wait at least 11ms after power-up (chapter 3.1)
	delay(20); 

#ifdef DEBUG
	log_debug( "Initializing SHT-11 pins\n");
#endif
	
	// Set up the SHT1x Data and Clock Pins
	SHT1x_InitPins();

#ifdef DEBUG
	log_debug( "Reset the SHT11\n");
#endif
	
	// Reset the SHT1x
	SHT1x_Reset();

	return(0);

}

int sample_sensor(void) 
{
	unsigned char noError = 1;  
	value humi_val,temp_val;

	log_debug( "starting measurement\n");
	
	// Request Temperature measurement
	noError = SHT1x_Measure_Start( SHT1xMeaT );
	if (!noError) {
		log_err( "could not start measurment, error\n");
		return(-1);
	}
	
	log_debug( "measurement complete, starting get value\n");
	
	// Read Temperature measurement
	noError = SHT1x_Get_Measure_Value( (unsigned short int*) &temp_val.i );
	if (!noError) {
		log_err( "Could not get value: error %d\n",(int)noError);
		return(-1);
	}

	log_debug( "start measure start for the humidity\n");
 
	// Request Humidity Measurement
	noError = SHT1x_Measure_Start( SHT1xMeaRh );
	if (!noError) {
		log_err( "could not get measurement: humidity %d\n",(int)noError);
		return(-1);
	}

	log_debug( "get measurment value\n");
		
	// Read Humidity measurement
	noError = SHT1x_Get_Measure_Value( (unsigned short int*) &humi_val.i );
	if (!noError) {
		log_err( "could not get measurement value: humidity %d\n",(int)noError);
		return(-1);
	}

	// Convert intergers to float and calculate true values
	temp_val.f = (float)temp_val.i;
	humi_val.f = (float)humi_val.i;

	log_debug( "calculating values\n");

	// Calculate Temperature and Humidity
	SHT1x_Calc(&humi_val.f, &temp_val.f);

	float c = temp_val.f;
	float f = ((c * 9.0) / 5.0) + 32.0; 
	float h = humi_val.f;

    log_debug( "Temp =  %.1f *C %.1f *F, Hum = %.1f \%\n", c, f, h);

  // some basic data validation
  if (f < 1.0) {
	log_err( "do not believe temp is less than 1 degree F ( %f ) \n",f);
	return(-1);
  }
  if (f > 150.0) {
    log_err( "do not believe temp is greater than 150 degree F ( %f ) \n",f);
    return(-1);
  }
  if (h < 1.0) {
    log_err( "do not believe humidity is less than 1 ( %f ) \n", h);
	return(-1);
  }
  if (h > 101.0) {
    fprintf(stderr, "do not believe humidity is greater than 100 ( %h ) \n", h);
    return(-1);
  }

  log_debug("getting time\n");

  // get current time - "canonical" (not-exactly-iso8601?)
  time_t now_time = time(NULL);
  struct tm now_tm = {0};
  localtime_r(&now_time, &now_tm);
  char now_str[40];
  strftime(now_str, sizeof now_str, "%FT%T%Z", &now_tm);

  log_debug( "writing history file\n");

  // update the history.yaml file
  // format is "sequence of maps" which looks like:
  // - { sensor: sname, time: XXXX, epoch: YYYYY, temperature: ZZ.Z, humidity: LL.L }

  FILE *fp = fopen(g_history_fn, "a");
  if (fp == NULL) { log_err("could not open history file\n"); return(-1); }

  fprintf(fp, "- { sensor: %s,time: \"%s\",epoch: %d, temperature: %.1f, celsius: %.1f, humidity: %.1f }\n",g_sensor_name,now_str,now_time,f,c,h);

  fclose(fp);

  // Also update the stats.yaml - and do the safe thing with writing to a temp then swapping the file
  // format: just sensor: sname\ntime: XXX .... as above

	log_debug("writing stats file\n");

  char tmpName[256];
  tmpnam(tmpName);
  fp = fopen(tmpName,"w");
  if (fp == NULL) {
    log_err( "could not open temp file for output %d\n", errno);
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
    log_err( "could not move to output file %d\n",errno);
    return(-1);
  }

  log_debug("successfully sampled\n");

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
  fprintf(stderr, " out-dir sensor-name data-pin clock-pin secs-delay\n");
  fprintf(stderr, "  the file 'sensor-name-history.yaml' will be updated,\n");
  fprintf(stderr, "  and sensor-name-stats.yaml with just the current,\n");
  fprintf(stderr, "  and it happens every secs-delay seconds (although if we get a GPIO error\n");
}


int main (int argc, char **argv)
{

	// get the command line sorted out
  	if (argc != 6) {
    	usage();
    	_exit(-1);
  	}

	char *out_dir = argv[1];
 	g_sensor_name = argv[2];
	g_data_pin = atoi(argv[3]);
	g_clock_pin = atoi(argv[4]);
	g_delay = atoi(argv[5]);

	log_err("starting log measurement on %s\n",g_sensor_name);
	log_debug("input parameters: out_dir %s sensor_name %s data_pin %d clock_pin %d delay %d\n",
		out_dir, g_sensor_name, g_data_pin, g_clock_pin, g_delay);

	if (0 != validate_pin(g_data_pin)) {
		log_err( "data pin %d is not a valid raspberry pi GPIO pin\n",g_data_pin);
		return(-1);
	}

	if (0 != validate_pin(g_clock_pin)) {
		log_err( "clock pin %d i snot a valid raspberry pi GPIO pin\n",g_clock_pin);
		return(-1);
	}

	snprintf(g_history_fn,sizeof(g_history_fn),"%s/%s-history.yaml",out_dir,g_sensor_name);
	snprintf(g_stats_fn,sizeof(g_stats_fn),"%s/%s-stats.yaml",out_dir,g_sensor_name);

	// init the chip
	if (!bcm2835_init()) {
		log_err(" could not init bcm2835, should never happen\n");
		return(-1);
	}

	if (0 != init_sensor()) {
		log_err( "init SHT1x sensor failed, no chip there? \n");
		_exit(-1);
	}

	do {
		sample_sensor();

		usleep( g_delay * 1000 * 1000 );

	} while (1);

	fprintf(stderr, "exiting...\n");
	return(0);
	
}
