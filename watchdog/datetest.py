#!/usr/bin/env python
import datetime
now = datetime.datetime.today()
print "the date is: {:%Y:%m:%d-%H:%M:%S-%z}".format(now)