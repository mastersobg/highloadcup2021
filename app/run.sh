#!/bin/bash

set -e

docker run -i --rm --memory=22g --cpus=4 -p 9000:9000 \
   --privileged --security-opt seccomp:unconfined --cap-add=ALL --network host  \
   --name highloadcup-app -t highloadcup-app
