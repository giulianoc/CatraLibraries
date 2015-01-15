#!/bin/sh
#
# Tracer Server configuration
#

TRACERSERVER_PATH=/h
export TRACERSERVER_PATH

TRACERSERVER_CONF_PATHNAME=/home/dev/usr_local/conf/TracerServer.cfg
export TRACERSERVER_CONF_PATHNAME

PATH=$PATH:/home/dev/usr_local/bin
export PATH

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/dev/usr_local/lib
export LD_LIBRARY_PATH

