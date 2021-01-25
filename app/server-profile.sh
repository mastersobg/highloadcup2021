#!/bin/bash

set -e

echo "Start profiling"
kill -12 `pidof highloadcup2021`

read -n 1 -s -r -p "Press any key to stop profiling"

kill -12 `pidof highloadcup2021`

echo -e "\nStopped"

PROF_FILE=`ls -tr highloadcup2021.prof.* | tail -n 1`
echo "Visualize $PROF_FILE"

IP=`hostname -i`
pprof -http $IP:9000 /app/highloadcup2021 /app/$PROF_FILE
