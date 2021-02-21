#!/bin/bash

set -e

docker run -i --rm  --memory=22g --cpus=4 -p 9000:9000 \
   --privileged --security-opt seccomp:unconfined --cap-add=ALL --network host  \
   -e ADDRESS=google.com -e Port=443 -e Schema=https \
   --name highloadcup-app -t highloadcup-app

   #-e ADDRESS=localhost -e Port=8000 -e Schema=http \
