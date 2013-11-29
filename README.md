CheeseCave
==========

CheeseCave

Two programs for measuring temp and humidity
temp_log_DHT and tmp_log_SHT, for two different sensors ---

The DHT22 is a low cost sensor available from AdaFruit industries and other sources.
It uses a one-wire protocol, is terrible at measuring humidity (unless we got a bad one)
and it tends to lock up after a few weeks of measurement. Also, about 20% of the time,
the protocol coming back from it can't be decoded - so you over sample. (this code should be written
to sample and sample until it works)

The SHT11 is a much more sophisticated device. It uses a two wire protocol,
samples much faster (no settling time), and the humidity doesn't fluctuate all over the place
(although we have not valdiated with a calibrated whether it is correct.

The temp_log code is written to run as a daemon, and sample constantly. Each
sensor has a different sensor name, so mulitple can be run, and outputs a "running history"
to one file, and updates the current state in another file. Further improvements could use RRD instead
of this JSON/YAML for history, which will be better for graphing.

We have wired our SHT11 to pins 16 and 18, so launching it is
