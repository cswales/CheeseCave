#!/usr/bin/python

# Brian Bulkowski , bbulkow
# Copywrite 2015 All rights reserved

# CHEESECAVE !

# This program takes the status information from the /var/lib/CheeseCave
# directory and displays it on an "Adafruit 16x2 Character LCD + Keypad"
# this solves the problem of having to go find your PC to check status.
# when you are right next to the 'cave or the network is down.

# The adafruit display is located at:
# https://learn.adafruit.com/adafruit-16x2-character-lcd-plus-keypad-for-raspberry-pi/overview

# when you put together the kit, be sure to buy the "long headers" This allows
# you to put your sensor and control wires on TOP of the plate.
# this particular kit is beneficial because it uses I2C, which means you
# still have enough pins for controlling your 'cave.

# The python library from Adafruit is used for control.
# git clone https://github.com/adafruit/Adafruit_Python_CharLCD.git
# and you will need to actually install the library into python
# with 'sudo python setup.py install'
# see the instructions

# this particular system is set up iwth the cheaper 1 color LCD plate.
# I have not yet determined how or weather to use the buttons for anything,
# as it seems you need to poll them. Doing some kind of wakeup would be good.



# Example using a character LCD plate.
import math
import time
import json
import os.path
import subprocess
import sys

import Adafruit_CharLCD as LCD

# where my config and control files are
# TODO: add config for this system in the config.json?
CONFIG_FILE_NAME = "/opt/pi/CheeseCave/config.json"
STATE_FILE_NAME  = "/var/lib/CheeseCave/sensor1-state.json"
RELAY_EXECUTABLE = "/opt/pi/CheeseCave/relay"
LOG_FILE_NAME    = "/var/lib/CheeseCave/control-log.json"

#
# Utilities from the CheeseCave system.
# TODO: refactor shared code in a library
#

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
    # print " load config data is ",data
    config_humidity_pin = data['config']['humidity_pin']
    # print "new humidity pin ",config_humidity_pin
    config_compressor_pin = data['config']['compressor_pin']
    # print "new compressor pin ",config_compressor_pin
    return data['config'], data['targets']

def config_has_changed():
    return os.path.getmtime(CONFIG_FILE_NAME) != config_mod_time

def get_current_sensors():
    state = open(STATE_FILE_NAME, 'r')
    data = json.load(state)
    return data

# returns "on" or "off"
def get_pin_state(pin):
    r =  subprocess.Popen(["sudo", RELAY_EXECUTABLE, "state",str(pin)], stdout=subprocess.PIPE).communicate()[0]
    # print " get pin state: pin ",pin," is ",r
    return r

def reverse( s ):
    if s == "off":
        return "on"
    else:
        return "off"

# set to "on" or "off"
def set_pin_state(pin, state):
    # print "setting pin ",pin," to ",state
    subprocess.Popen(["sudo", RELAY_EXECUTABLE, state, str(pin)])

def get_compressor_state():
    if config_compressor_pin == 0 :
        return "off"
    if config_compressor_pin == 0 :
        return "off"
    r = get_pin_state(config_compressor_pin)
    # print "get compressor state: ",r
    return r

def set_compressor_state(state):
    if config_compressor_pin == 0 :
        return
    log_state('compressor',state)
    set_pin_state(config_compressor_pin, state)

# the polarity of the humidity pin is backward
def get_humidity_state():
    if config_humidity_pin == 0 :
        # print " can't read humidity pin not configured "
        return "off"
    r = reverse ( get_pin_state(config_humidity_pin ))
    # print " get humidity state: ",r
    return r

def set_humidity_state(state):
    if config_humidity_pin == 0 :
        return
    log_state('humidity', state)
    state = reverse( state )
    set_pin_state(config_humidity_pin, state)



# Initialize the LCD using the pins 
lcd = LCD.Adafruit_CharLCDPlate()
lcd.clear()
# backlight on
lcd.set_color(1.0, 1.0, 1.0)
lcd.message('INITIALIZING\n')

config, targets = load_config()

while True:

	# Get the new current temp and humidity values from the file
	sensors = get_current_sensors()

	if config_has_changed():
		config, targets = load_config()

	# write the temp and humidity
#	msg = 'TEMP: {0}\nCur {1:.1f}  Set {2:.1f}\n'.format( get_compressor_state(), sensors['temperature'], targets['temperature'] )
	msg = 'TEMP:  Cur {1:.1f}  \n{0} Set {2:.1f}   \n'.format( get_compressor_state(), sensors['temperature'], targets['temperature'] )
#	lcd.clear()
	lcd.set_cursor(0,0)
	lcd.message( msg )
	time.sleep(2.0)

	msg = 'HUMID: Cur {1:.1f}  \n{0} Set {2:.1f}   \n'.format( get_humidity_state(), sensors['humidity'], targets['humidity'] )
	lcd.set_cursor(0,0)
	lcd.message( msg )
	time.sleep(2.0)

	# write the last time updated




