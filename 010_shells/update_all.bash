#!/bin/bash

for dir in */ ; do
  if [ -d "$dir/.git" ]; then
    echo "Updating $dir"
    cd "$dir"
    git pull
    cd ..
  fi
done