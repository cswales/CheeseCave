CheeseCave
==========

# Cookbook

## Prerequisites

Which broadcome libraries and such are required

Which distributions does this work on

## Hardware -- sensor

Explain which sensor and how it is wired

## Hardware -- fridge control

Explain the relay circuit, show a picture, and
how it is wired

## Make

Makes the 'relay' and 'temp_log_DHT'

## Install

Server, relay, temp_log_DHT

## Data files by default

# Overview

This project controls a fridge. Right now, it simply controls the temperature,
and provides a webpage for you to see the status. It also records temp and humidity
into a datafile.

This project has several components.

The "temp_log" is a system for measuring temperature with the Raspberry Pi.

When I started on this, I found that there was "sample code" to measure temp
with several sensors, but nothing that would run long-term, and was not
outputting data formats that are easily used by modern web developers.

Thus, we created "temp_log_SHT", and "temp_log_DHT". They are written in C,
meant to be run as a long-term daemon ( use 'supervisord' to keep them alive ).

You can set the Raspberry Pi pins that the sensor is at
You can set the logfile location, the history file output location, the current status file

The DHT is for the DHT-11 through DHT-22 series of sensors. We bought ours from AdaFruit
technology. We found this sensor / code is UNSTABLE for long term operation - the
sensor tends to "lock up" (stop sampling) after several days. The only way to "unstick"
is to remove power from the SENSOR (not the pi!) and reapply it. Although this could be done
with a GPIO pin, we abandonded this (fairly cheap) sensor in favor of the DHT-11 series.

The DHT-11 is a more sophisticated sensor, and twice the price (again, at Adafruit).
So far, after several weeks of running, the DHT-11 has shown no signs of sensor problems,
and we have more belief that the humidity is correct, although this has not been validated.

# The controller

In order to read the temp / humidity sensor and turn on and off a fridge, we have
the controller program. This is also C and is a daemon. It expects a single GPIO pin
to be wired to a transistor and a relay. It looks at a YAML config file, and monitors
the "status" file created by the "temp_log" program. It is meant to be run as a daemon
constantly, and does some file output.

# The web page

We have written a small python web server which displays the current status of the cave.

It allows changing of the YAML config file, viewing of the current sensor values. Because
this actually changes the temp of your fridge, it should be protected somehow by login --- we haven't done that!

There is also a component which takes a "snapshot" of the raspberry pi camera and displays it.
The idea was to view the status of the cheese (or take time lapse!) of the cheeses without
opening the fridge. We've realized this is a little dumb, because there's just a bunch of boxes
in the fridge and you can't see any detail (we do have some lights that we could also control).
We also realized that the cable that comes with the 'pi is too short, so we'd switch to using
a USB camera instead of the pi camera, because the higher bandwidth PI camera doesn't matter for
the occasional snapshot.

# More instructions

We have a google doc that includes all the information we've been building up regarding
the different software that needs to be installed, how to run these programs, design choices made,
all kinds of things. It's not in final form, but you can write us at this account ( or email brian@bulkowski.org )
and ask questions / ask us to share the current document.

# Data visualization & conversion

Our old YAML data format was rough and wrong. And, YAML just ain't what is used to be.

Thus, I've included two new scripts, written in Go.

```
go run go/yaml-csv.go -i inputfile -o outputfile
go run go/yaml-json.go -i inputfile -o outputfile
```

These files will convert into both JSON and CSV, dropping some of the datapoints
as spurrious if configured to do so. You will need Go, which is now widely available.
It has been tested with Go 1.4 and runs on a 50,000 line file in far less than a minute.
Of course I should make these one file, with an inputformat and outputformat specification,
but I'm lazy.

Once you have a CSV file, it's easy to use outside services. I found that http://plot.ly/ was
particularly simple to use. It's very easy to upload the file, click on "time" vs "humidity" and "humidity" to get a nice little plot.

However, there's no zoom in / zoom out that you'd really want for this kind of data. ( Like Cube gives you). Any suggestions apprecated.


# What's next ???

1) Switch the data format to streaming JSON instead of YAML. 
First, we're using YAML wrong.
Second, JSON has come up with standards for this kind of streaming data

2) Ability to load this data into a visualization system like Mixpanel. Maybe even real-time
push the data up.

3) Use a small display to show the current status of a Cave. We find that if the temp
or humidity is wrong, it's a pain to go to the website. Also, our connectivity to the 'cave subnet
is a little flakey.

4) Better instructions. We've wired the sensors and relays in particular ways --- which work ---
and we'd like to share our expertise.

5) Humidity control.

6) Better code structure for the "temp_log". THis should be one executable, with different programs
to measure the values. This means multithreading everything, too.

7) Better interaction with the SHT sensor. The code that I found (correct license included in source)
is interacting with the sensor in a pretty dumb way - reading the temp, then the humidity, when the
sensor supports reading them both at the same time. Also, there is a hardcoded value for the voltage
of the part, which should be exposed to the command line.

8) Better makefile. I took a shortcut where 'make.sh' make temp_log_SHT, and 'Makefile' makes temp_log_DHT.

# More information about sensors

The DHT22 is a low cost sensor available from AdaFruit industries and other sources.
It uses a one-wire protocol, is terrible at measuring humidity (unless we got a bad one)
and it tends to lock up after a few weeks of measurement. Also, about 20% of the time,
the protocol coming back from it can't be decoded - so you over sample. (this code should be written
to sample and sample until it works)

The SHT11 is a much more sophisticated device. It uses a two wire protocol,
samples much faster (no settling time), and the humidity doesn't fluctuate all over the place
(although we have not valdiated with a calibrated whether it is correct.
