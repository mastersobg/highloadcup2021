#!/bin/bash

set -e

docker run -i --rm  --memory=2g --cpus=4 -p 9000:9000 \
   --network host  \
   -e ADDRESS=localhost -e Port=8000 -e Schema=http \
   --name highloadcup-app -t highloadcup-app

#docker run -i --rm  --memory=2g --cpus=4 -p 9000:9000 \
#   --privileged --security-opt seccomp:unconfined --cap-add=ALL --network host  \
#   -e ADDRESS=localhost -e Port=8000 -e Schema=http \
#   --name highloadcup-app -t highloadcup-app

