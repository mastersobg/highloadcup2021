#!/bin/bash

set -e

COMMIT_HASH=`git log -1 --format=%H`
docker build --progress=plain  -f Dockerfile -t highloadcup-app --build-arg build_type=$1 --build-arg commit_hash=$COMMIT_HASH .
