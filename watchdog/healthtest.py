#!/usr/bin/env python

import subprocess
import sys
import json

from datetime import timedelta
from datetime import datetime

config_hosts = [ "cave1", "notfound" ]
config_user = "pi"
config_key_file = "id_cheesecave"

# read the last line of a file
config_history_file = "/var/lib/CheeseCave/sensor1-history.json"

def validate_history( history ):
	i = 0
	for key, value in history.iteritems():
		print('entry {} {} {}'.format(i, key, value) )
		i = i + 1
	print '\n\n'

	if not "temperature" in history:
		r_str = "Warning! object has no temperature, corrupted"
		print r_str
		return False, r_str

	if history["temperature"] > 80:
		r_str = "warning ! high temperature {}".format(history.temperature)
		print r_str
		return False, r_str
	else:
		print(" awesome, temp is in range ")

	if not "humidity" in history:
		r_str = "Warning! object has no humidity, corrupted"
		print r_str
		return False, r_str

	if not "time" in history:
		r_str = "Warning! object has no time, corrupted"
		print r_str
		return False, r_str

	limit = datetime.now() - timedelta( seconds=30 )
	t = datetime.strptime(history["time"],"%Y-%m-%dT%H:%M:%S%Z")
	if t < limit:
		r_str = "Warning! Time string is too old, hung cave???"
		print r_str
		return False, r_str
	else:
		print "Awesome! Time is up to date"

	if not "epoch" in history:
		r_str = "Warning! object has no Epoch!, corrupted"
		print r_str
		return False, r_str

	if datetime.fromtimestamp( history["epoch"] ) < limit :
		r_str = "Warning! Epoch is too old, hung cave???"
		print r_str
		return False, r_str
	else:
		print "Awesome! Epoch is up to date"

	return True, ""

def health_check( host ):

	print( ' attempting to read from host {} file {}'.format(host, config_history_file) )

	cmd = ['ssh', "-i", config_key_file, '{}@{}'.format(config_user, host), "tail -1 {}".format(config_history_file) ]

	ssh = subprocess.Popen(cmd, shell = False, stdout=subprocess.PIPE, stderr=subprocess.PIPE )

	result = ssh.stdout.readlines()
	if result == []:
		error = ssh.stderr.readlines()
		print "ERROR: " + error[0]
		return False, error[0]
		
	else:
		print "SUCCESS: " + result[0]
		ret_obj = json.loads( result[0] )
		return ( validate_history( ret_obj ) )
		

for host in config_hosts:
	success, result_str = health_check(host)
	if success != True:
		print(" FAIL FOR HOST",host," error string ",result_str)
	else:
		print(" succeeded with host",host)

print "DONE"