#!/bin/bash

set -e

if [[ -n $(git status --porcelain) ]]; then
	git status --porcelain
	echo "repo is dirty, proceed?";
	read
fi

./build.sh release

tag=`git rev-parse --short HEAD`

git tag $tag
git push
git push origin --tags

docker tag highloadcup-app stor.highloadcup.ru/rally/butterfly_builder
docker push stor.highloadcup.ru/rally/butterfly_builder

