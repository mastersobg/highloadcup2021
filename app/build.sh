#!/bin/bash

set -e

COMMIT_HASH=`git log -1 --format=%H`
docker build --progress=plain  -f Dockerfile-build -t highloadcup-build --build-arg build_type=$1 --build-arg commit_hash=$COMMIT_HASH .

docker run -i --rm --name highloadcup-build -v $(pwd)/out:/app/build -t highloadcup-build

docker build --progress=plain  -f Dockerfile -t highloadcup-bin --build-arg build_type=$1 --build-arg commit_hash=$COMMIT_HASH .
