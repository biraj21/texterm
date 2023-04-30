#!/bin/bash

set -e

bin=texterm
path="$HOME/.$bin/bin"

echo "creating executable..."
make "$bin"

echo "creating path if it doesn't already exists..."
mkdir -p "$path"

echo "removing old executable..."
rm -f "$path/$bin"

echo "moving new executable to path..."
mv "$bin" "$path"

echo "changing executable file permissions..."
chmod 775 "$path/$bin"

echo "creating a symbolic link..."
ln -fs "$path/$bin" "$HOME/.local/bin/"

echo "done!"
