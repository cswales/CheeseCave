#!/usr/bin/python

import json
import os.path
import time
import subprocess
import sys

CONFIG_FILE_NAME = "/opt/pi/CheeseCave/config.json"
STATE_FILE_NAME  = "/var/lib/CheeseCave/sensor1-state.json"
RELAY_EXECUTABLE = "/opt/pi/CheeseCave/relay"
LOG_FILE_NAME    = "/var/lib/CheeseCave/control-log.json"

config_mod_time = 0
config_compressor_pin = 0
config_humidity_pin = 0

def load_config():
	global config_mod_time
	global config_compressor_pin
	global config_humidity_pin
	with open(CONFIG_FILE_NAME, 'r') as f:
		data = json.load(f)
		config_mod_time = os.path.getmtime(CONFIG_FILE_NAME)
	print " load config data is ",data
	config_humidity_pin = data['config']['humidity_pin']
	print "new humidity pin ",config_humidity_pin
	config_compressor_pin = data['config']['compressor_pin']
	print "new compressor pin ",config_compressor_pin
	return data['config'], data['targets']

def config_has_changed():
	return os.path.getmtime(CONFIG_FILE_NAME) != config_mod_time

def log_state(relay, state):
	with open(LOG_FILE_NAME, 'a') as log_file:
		log_file.write('{"epoch":%d, "relay":"%s", "state":"%s"}\n'%(int(time.time()), relay, state))

# returns "on" or "off"
def get_pin_state(pin):
	r =  subprocess.Popen(["sudo", RELAY_EXECUTABLE, "state",str(pin)], stdout=subprocess.PIPE).communicate()[0]
	return r

def reverse( s ):
	if s == "off":
		return "on"
	else:
		return "off"	

# set to "on" or "off"
def set_pin_state(pin, state):
	print "setting pin ",pin," to ",state
	subprocess.Popen(["sudo", RELAY_EXECUTABLE, state, str(pin)])

def get_compressor_state():
	if config_compressor_pin == 0 :
		return "off"
	return get_pin_state(config_compressor_pin)
	#return ["on"]

def set_compressor_state(state):
	if config_compressor_pin == 0 :
		return
	log_state('compressor',state)
	set_pin_state(config_compressor_pin, state)

# the polarity of the humidity pin is backward
def get_humidity_state():
	if config_humidity_pin == 0 :
		return "off"
	return reverse ( get_pin_state(config_humidity_pin) )

def set_humidity_state(state):
	if config_humidity_pin == 0 :
		return
	log_state('humidity', state)
	state = reverse( state )
	set_pin_state(config_humidity_pin, state)

def get_current_sensors():
	state = open(STATE_FILE_NAME, 'r')
	data = json.load(state)
	return data

if __name__ == "__main__":
	first_time = True
	compressor_end = 0
	humidity_end = 0

	config, targets = load_config()

	# special case: 
	if get_compressor_state() == 'on' :
		print "turning off compressor for initial state"
		set_compressor_state('off')

	if get_humidity_state() == 'on' :
		print "turning off humidity for initial state"
		set_humidity_state('off')

	while True:
		# get configuration if we need to
		if (config_has_changed() or first_time):
			first_time = False
			config, targets = load_config()
			print "config ",config
			print "targets ",targets

		# get relay state(s)
		compressor = get_compressor_state()
		print "compressor state (",compressor,")"
		humidity = get_humidity_state()
		print "humidity state (",humidity,")"


		# get current sensor values (temperature, humidity)
		sensors = get_current_sensors()
		print "sensors ",sensors

		# modify compressor relay based on desired conditions,
		# current stats, and relay state.
		# For the moment, we only do the compressor relay
		if compressor_end > 0 :
			if time.time() > compressor_end :
				if sensors['temperature'] > targets['temperature']:
					# add another quanta of time without bumming out compressor
					compressor_end = time.time() + config['temperature_delay']
					print "adding more time to the compressor end"
				else:
					set_compressor_state("off")
					compressor_end = 0
					print "turning off compressor after delay: temp",sensors['temperature']
			else :
				print "compressor staying on"

		elif ( sensors['temperature'] > targets['temperature'] ) and ( compressor == 'off' ) :
			set_compressor_state('on')
			compressor_end = time.time() + config['temperature_delay']
			print time.strftime("%b %d %Y %I:%M%p %Z: turned temp relay ON")
			sys.stdout.flush()	

		else:
			print " compressor relay is (",compressor,")"
			print " sensor is (",sensors['temperature'],")"
			print " target temp is (",targets['temperature'],")"
			print "no temp change needed"

		# modify humidity relay based on desired conditions,
		# current current value, and relay state.
		# Only do if we have humidity configured.
		if config_humidity_pin :
			if humidity_end > 0 :
				if time.time() > humidity_end :
					if sensors['humidity'] < targets['humidity']:
						# add another quanta of time without bumming out device
						humidity_end = time.time() + config['humidity_delay']
						print "adding more time to the humidity end"
					else:
						set_humidity_state('off')
						humidity_end = 0
						print "turning off humidifier after delay: humid",sensors['humidity']
				else :
					print "humidifier staying on"

			elif (sensors['humidity'] < targets['humidity']) and humidity == 'off':
				set_humidity_state('on')
				humidity_end = time.time() + config['humidity_delay']
				print time.strftime("%b %d %Y %I:%M%p %Z: turned humidity ON")
				sys.stdout.flush()
		
			else:
				print " humidity relay is (",humidity,")"
				print " humidity value is (",sensors['humidity'],")"
				print " target humidity is (",targets['humidity'],")"
				print "no humidity change needed"

		time.sleep(config['sleep_delay'])
