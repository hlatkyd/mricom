# mricom

-- WORK IN PROGRESS --

Physiological data acquisition and session control program to support Vnmrj on
Agilent MRI scanner console host machine. Data acquisition is handled by a NI
PCI card (PCI-6035E) on the device host machine running *mricom* and its
companion background process, *mribg*.

## Build & Install

Tested on Ubuntu 18.04

Install prerequisites:

`sudo apt-get -y install libcomedi-dev libreadline-dev kst`

Build from source:

`make all`

Install manually


## Setup on console host 

*mricom* works in conjunction with modified Vnmrj pulse sequences. The
subprogram *ttlctrl* on the device host machine communicates with the console
during each sequence.  This consists of:

1. A simple TTL handshake between *ttlctrl* and console
2. *ttlctrl* signals sequence start for console
3. console signals sequence end to *ttlctrl*

On the console side two connectors are needed on the user interface panel.
Sequences can use the pulse sequence commands `xgate` and `spXon` / `spXoff`.

An example modification of the pulse sequence c code to include the TTL
handshake using user output 1 and the wait for *ttlctrl* signal at the
beginning of the pulse sequence:

```c
sp1on();
xgate(1);       // wait for one TTL high on user gate input
sp1off();
delay(5e-3);    // wait 5 microseconds
sp1on();        // ready for sequece start

xgate(1);       // wait for sequence start signal from ttlctrl
sp1off();
```

Signaling at the end of the pulse sequence:

```
sp1on(); delay(5e-3); sp1off();
```

*ttlctrl* is not constantly running on the device host, but it is called each
time before a sequence starts. On the console host running Vnmrj, another
subprogram, *vnmrclient*, sohuld be called preferably by a Vnmrj magical macro
preceding the launch of the pulse sequence. *vnmrclient* in turn communicates
with *mribg* on the device host via TCP, and mribg calls for *ttlctrl* to
control the timing of upcoming pulse sequence.

## Usage

none yet
