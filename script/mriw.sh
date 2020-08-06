#!/bin/bash

# Monitor mricom in a terminal window via repeatedly calling the mrimon utility. 
# Displays: mribg status (running, waiting, etc), current processes (analogdaq
# blockstim, ttlctrl, etc.) study ID and completed sequences. 
#
#
#

rootdir=$MRICOMDIR
path="($rootdir)/bin/mrimon"

watch -n 0.5 --color 'unbuffer /home/david/dev/mricom/bin/mrimon'
