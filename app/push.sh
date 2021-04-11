#!/bin/bash

set -e

if [[ -n $(git status --porcelain) ]]; then
	git status --porcelain
	echo "repo is dirty, proceed?";
	read
fi

rm -rf out/*
./build.sh release

tag=`git rev-parse --short HEAD`

git tag $tag
git push
git push origin --tags

docker tag highloadcup-bin stor.highloadcup.ru/rally/white_quetzal
docker push stor.highloadcup.ru/rally/white_quetzal

