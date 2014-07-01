## Description

This is a simple proxy daemon that allows you to forward TCP requests hitting a specified port on the localhost to a different port on another host. It is written in ANSI C so it takes a very little space and can be used on embedded devices.

## Installation

On Linux compile the software using "make". On Windows use "make" from Cygwin (http://cygwin.com).

To build an executable for Tomato firmware (http://www.polarcloud.com/tomato) you need to have the mipsel toolchain installed. Fetch it with git:
```
git clone git://repo.or.cz/tomato.git tomato
```
and follow instructions in the "tools/README.TXT" file.

Next, build proxy for Tomato with:
```
make tomato
```

## Usage

Command line syntax goes as follows:
```
proxy -l local_port -h remote_host -p remote_port [-i "input parser"] [-o "output parser"]
```
Suppose you want to open port 8080 on a public host and forward all TCP packets to port 80 on machine 192.168.1.2 in the local network. In this case you install proxy on a public host and run it the following command:
```
proxy -l 8080 -h 192.168.1.2 -p 80
```
Input parser and output parser are commands through which incoming and outgoing packets can be forwarded. For example to use a "tee" command to log all incoming http data to incoming.txt file you can start proxy with the following options:
```
proxy -l 8080 -h 192.168.1.2 -p 80 -i "tee -a incoming.txt"
```
The parser command will receive data from socket to its standard input and should send parsed data to the standard output. It should also flush its output at a reasonable rate to not withhold network communication.

## Advanced usage

You can use input and output parsers to copy incoming / outgoing traffic to other hosts (e.g. for monitoring reasons). Just use "tee" command to copy network packets to named pipes and then read from those pipes.

The following scenario assumes you set up the program on local port 8080 as a proxy to www.wikipedia.org, and copy all outgoing requests to servers listening on ports 9000 and 9001 on localhost.

First, create the listeners (each one in separate terminal window):
```
while true; do nc -l 9000; done
while true; do nc -l 9001; done
```
Next, create named pipes "fifo0" and "fifo1":
```
mkfifo fifo0
mkfifo fifo1
```
Next, start reading data in loop from the pipes and forwarding it to the listeners (run these commands in separate terminals also):
```
while true; do cat fifo0 | nc localhost 9000; done
while true; do cat fifo1 | nc localhost 9001; done
```
Last, set up the proxy:
```
proxy -l 8080 -h www.wikipedia.org -p 80 -o "tee fifo0 fifo1"
```
Now when you send a request through the proxy, you should see it on all terminals where you set up the listeners:
```
curl -i http://localhost:8080
```
**Important notice:** Make sure to read data from all the pipes. If you don't, tee will stuck on writing to the pipe which is not being read from, and so the proxy will be stuck also. Also if you restart a listener, netcat may not reconnect to it, which means that you will need to restart the appropriate forwarder script, too.
