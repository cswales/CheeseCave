#!/usr/bin/python

import yaml
import os.path
import time
import subprocess
import sys

CONFIG_FILE_NAME = "/opt/pi/CheeseCave/config.yaml"
STATS_FILE_NAME  = "/var/lib/CheeseCave/sensor1-stats.yaml"
RELAY_EXECUTABLE = "/opt/pi/CheeseCave/relay"
LOG_FILE_NAME    = "/var/lib/CheeseCave/control.yaml"

config_mod_time = 0

def load_config():
	global config_mod_time
	with open(CONFIG_FILE_NAME, 'r') as f:
		data = yaml.load(f)
		#print yaml.dump(data)
		config_mod_time = os.path.getmtime(CONFIG_FILE_NAME)
	return data['targets']

def config_has_changed():
	return os.path.getmtime(CONFIG_FILE_NAME) != config_mod_time

def log_state(state):
	with open(LOG_FILE_NAME, 'a') as log_file:
		log_file.write("-{epoch:%d, state:%s}\n"%(int(time.time()), state))

def get_relay_state():
	#return ["on"]
	return [subprocess.Popen(["sudo", RELAY_EXECUTABLE, "state"], stdout=subprocess.PIPE).communicate()[0]]

def set_relay_state(state):
	#pass
	subprocess.Popen(["sudo", RELAY_EXECUTABLE, state])

def get_current_stats():
	stats = open(STATS_FILE_NAME, 'r')
	data = yaml.load(stats)
	#print yaml.dump(data)		
	return data

if __name__ == "__main__":
	first_time = True
	while True:
		# get configuration if we need to
		if (config_has_changed() or first_time):
			first_time = False
			config = load_config()
			#print config

		# get relay state(s)
		relays = get_relay_state()

		# get current stats (temperature, humidity)
		stats = get_current_stats()

		# modify relay based on desired conditions,
		# current stats, and relay state.
		# For the moment, we only do the temperature 
		# relay
		if ((stats['temperature'] > config['temperature'] + 
			(0.5 * config['temperature_range'])) and 
			relays[0] == 'off'):
			set_relay_state("on")
			log_state("on")
			print time.strftime("%b %d %Y %I:%M%p %Z: turned relay ON")
			sys.stdout.flush()	
		elif ((stats['temperature'] < config['temperature'] -
			(0.5 * config['temperature_range'])) and 
			relays[0] == 'on'):
			set_relay_state("off")
			log_state("off")
			print time.strftime("%b %d %Y %I:%M%p %Z: turned relay OFF")
			sys.stdout.flush()	
		else:
			pass
			#print "no changes needed"	
		time.sleep(5)
