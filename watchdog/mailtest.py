#!/usr/bin/env python

# This is a test program to send GMAIL.
# FROM: http://naelshiab.com/tutorial-send-email-python/

# THIS WORKS, but you need to turn on "allow insecure app access"
# to your account, and you have to put your password right here
# in this file. That's kind of terrible! Maybe would be less bad
# if we literally created a new account for "cheesecave" or "950arnold"
# or something. YES. We will do that.
# To do this for yourself, you'll go to myaccount.google.com 
# ( at least in 2016 )

import smtplib
from email.MIMEMultipart import MIMEMultipart
from email.MIMEText import MIMEText

 
from_addr = "950.arnold.way@gmail.com"
from_password = "mJmE76Js"

to_addr = "brian@bulkowski.org,cswales@gmail.com"

msg = MIMEMultipart()
msg['From'] = from_addr
msg['To'] = to_addr
msg['Subject'] = "Hello Brian! this is a test"
 
body = "I am trying to test whether sending through gmail works. \n"
body += "this is the second line of the message \n"

msg.attach(MIMEText(body, 'plain'))
 
server = smtplib.SMTP('smtp.gmail.com', 587)
server.starttls()
server.login(from_addr, from_password)
text = msg.as_string()
server.sendmail(from_addr, to_addr, text)

server.quit()