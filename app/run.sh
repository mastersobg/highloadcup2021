#!/bin/bash

set -e

ADDRESS="172.17.0.1"

if [ "$(uname)" == "Darwin" ]; then
  ADDRESS="host.docker.internal"
fi

docker run -i --rm  --memory=2g --cpus=4 -p 9000:9000 -p 1234:1234 \
   --privileged --security-opt seccomp:unconfined --cap-add=ALL \
   -e ADDRESS=$ADDRESS -e Port=8000 -e Schema=http \
   --name highloadcup-bin -t highloadcup-bin


