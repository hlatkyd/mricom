mricom
------
### WORK IN PROGRESS ###
Physiological data acquisition and session control program to support Vnmrj on
Agilent MRI scanner console host machine. Data acquisition is handled by a 
NI PCI card (PCI-6035E). Procpar files and console user interface triggers are
monitored, and user defined triggers are generated to interface with custom
stimulation hardware.

Install
-----
Tested on Ubuntu 18.04

Install prerequisites:
```
sudo apt-get -y install libcomedi-dev libreadline-dev kst
```

Build from source:
```
make all
```
Install manually

Usage
-----
none yet
