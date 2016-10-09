# Watchdog

When you are fooling with embedded systems like this which control real hardware, and have real repercussions when they stop functioning for any reason, it is pleasant to have an external system monitoring all your systems.

In this project, I intend to use a "pull method". In this design, the watchdog - running on an external machine which has higher availability - will be polling all the embedded devices, and if it doesn't reach them, it will complain.

An alternate device is to have the embedded systems themselves use a standard service and protocol - such as collect.d - and report to an external service. THere are benefits in this, such as professionally maintained alterting systems, but I will leave that for another day and project.

## Polling

First attempt will be to use SSH to remote-exec a command like 'tail -n1' to get the last line of a log file. As each of these files are now JSON files, this will give an object that can be serialized and examined. These tend to have timestamps, and they tend to have values. 

This kind of polling leads to three kinds of errors:

1. Can't reach remote endpoint

1. Endpoint is not updating its files

1. Data coming back from endpoint is out of range ( temp too high )

## Polling with SSH and Python

When you're logging into a remote machine, you've got three choices.

1. Create an account with no password and has just the privs you want

2. Use the package 'sshpass' which will pass a password to SSH

3. User key access instead of passwords.

## Using keys

Create a key pair. There is a keypair checked in here - kind of like checking in a password.

For the machine you will log into, append the public key to "authorized keys", in a particular user's .ssh directory.

When you need to log in, use 'ssh -i <private key> user@host'

That's about as simple as it gets, bob !

## Alerting

Two methods of alerting are considered: email and SMS.

For email, one typically uses a password controlled SMTP server. As most people have gmail, and I use gmail, I will investigate using Gmail with username and password auth, from Python.

For SMS, a very common system is a free twilio account. An account like this is reputed to also have great properties: even easier to code, and comes at you as a text message.

