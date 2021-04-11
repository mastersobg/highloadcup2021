#!/bin/bash

set -e

docker exec -it highloadcup-bin /bin/bash -c "./server-dump.sh"
