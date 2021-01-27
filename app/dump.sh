#!/bin/bash

set -e

docker exec -it highloadcup-app /bin/bash -c "./server-dump.sh"
