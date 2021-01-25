#!/bin/bash

set -e

docker build --progress=plain  -f Dockerfile -t highloadcup-app --build-arg build_type=$1 .
