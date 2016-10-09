#!/usr/bin/env python

# Config is done through a local JSON file
import json
import os.path

# The watchdog will send messages if it finds things it doesn't like.
# Grrrr! Grrr!
import smtplib
from email.MIMEMultipart import MIMEMultipart
from email.MIMEText import MIMEText

from_addr = "950.arnold.way@gmail.com"
from_password = "mJmE76Js"

def send_alert():

	msg = MIMEMultipart()
	msg['From'] = from_addr
	msg['To'] = config_to_addr
	msg['Subject'] = "ALERT about a CHEESECAVE"
	 
	body = "Hi Brian and Carolyn! \n"
	body += 'This is an ALERT a test which was sent on {}\n'.format()

	msg.attach(MIMEText(body, 'plain'))
	 
	server = smtplib.SMTP('smtp.gmail.com', 587)
	server.starttls()
	server.login(from_addr, from_password)
	text = msg.as_string()
	server.sendmail(from_addr, to_addr, text)

	server.quit()

##
## Here is health check code
##
## Parameter is host, username, password
##
## Returns a NULL if healthy
## Returns a string to send if there is a problem

def health_check(host, user, password):

	return null


##
## here is a config file
## 

CONFIG_FILE_NAME = "config.json"

# hosts separated by commas
config_hosts = "pi1"
config_to_addr = "brian@bulkowski.org,cswales@gmail.com"

# username and password for each host. Currently consider they must be the same
config_username = "pi"
config_password = "paranoia"

# todo: would be nice to have a flexible system for different kinds of hosts
# having different kinds of health checks.
# today we have only one kind of host, with one kind of status,
# so we don't need any of that complexity

def load_config():
	global config_hosts
	global config_to_addr
	global config_username
	global config_password

	with open(CONFIG_FILE_NAME, 'r') as f:
		data = json.load(f)
		config_mod_time = os.path.getmtime(CONFIG_FILE_NAME)