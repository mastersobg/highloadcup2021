#!/bin/bash

set -e

#dmesg -C

cat /proc/cpuinfo | grep "model name"

echo "first argument" $1

case $1 in
  profile-cpu)
    LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libprofiler.so env CPUPROFILE=/app/highloadcup2021.prof CPUPROFILESIGNAL=12 \
      ./highloadcup2021 || echo "Crashed"
    ;;
  profile-memory)
    LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libtcmalloc.so env HEAPPROFILE=/app/highloadcup2021.prof HEAPPROFILESIGNAL=12 \
      HEAP_PROFILE_ALLOCATION_INTERVAL=107374182400 HEAP_PROFILE_INUSE_INTERVAL=107374182400 ./highloadcup2021 || echo "Crashed"
    ;;
  lock)
    mutrace --hash-size=10007 ./highloadcup2021 || echo "Crashed"
    ;;
  *)
    ./highloadcup2021 2>&1 || echo "Crashed"
    ;;
esac

dmesg -T | grep -E -i -B10 'killed process'