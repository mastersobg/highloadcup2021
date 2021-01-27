#!/bin/bash

set -e

gdb -q -p `pidof highloadcup2021` <<< "thread apply all bt"
