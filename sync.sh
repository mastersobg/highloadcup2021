#!/bin/zsh

source ~/.zshrc

rsync -arzv --delete --exclude=.idea --exclude=CMakeCache.txt --exclude=cmake-build-debug \
 --exclude=highloadcup2021.cbp --exclude=CMakeFiles --exclude=Makefile --exclude=Testing \
 --exclude=build --exclude=cmake_install.cmake /Users/gorbachev/sources/github/highloadcup2021/ build3:~/hlc
