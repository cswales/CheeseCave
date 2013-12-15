CheeseCave
==========

CheeseCave

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

The controller

In order to read the temp / humidity sensor and turn on and off a fridge, we have
the controller program. This is also C and is a daemon. It expects a single GPIO pin
to be wired to a transistor and a relay. It looks at a YAML config file, and monitors
the "status" file created by the "temp_log" program. It is meant to be run as a daemon
constantly, and does some file output.

The web page

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

More instructions

We have a google doc that includes all the information we've been building up regarding
the different software that needs to be installed, how to run these programs, design choices made,
all kinds of things. It's not in final form, but you can write us at this account ( or email brian@bulkowski.org )
and ask questions / ask us to share the current document.

What's next ???

Our next extensions of this project are:
1) HUMIDITY CONTROL. In order to get better cheese ripening, we need humidity control, which
means being able to pipe air in for air flow (decreasing humidity), which we've decided to use a
aquarium air pump and control through the pi, and a ultrasonic mister to add humidity. These
parts are on order.
2) Better instructions. All of these components are pointing different places, and our
basic instructions of installing aren't that clear.
3) Better code structure for the "temp_log". THis should be one executable, with different programs
to measure the values. This means multithreading everything, too.
4) Better interaction with the SHT sensor. The code that I found (correct license included in source)
is interacting with the sensor in a pretty dumb way - reading the temp, then the humidity, when the
sensor supports reading them both at the same time. Also, there is a hardcoded value for the voltage
of the part, which should be exposed to the command line.
5) Better makefile. I took a shortcut where 'make.sh' make temp_log_SHT, and 'Makefile' makes temp_log_DHT.

More information about sensors

The DHT22 is a low cost sensor available from AdaFruit industries and other sources.
It uses a one-wire protocol, is terrible at measuring humidity (unless we got a bad one)
and it tends to lock up after a few weeks of measurement. Also, about 20% of the time,
the protocol coming back from it can't be decoded - so you over sample. (this code should be written
to sample and sample until it works)

The SHT11 is a much more sophisticated device. It uses a two wire protocol,
samples much faster (no settling time), and the humidity doesn't fluctuate all over the place
(although we have not valdiated with a calibrated whether it is correct.
