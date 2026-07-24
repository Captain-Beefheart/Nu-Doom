#!/usr/bin/env bash
set -e
cd /c/Users/computer/CODE/doomgeneric/doomgeneric
mkdir -p ../dist
cp doomgeneric.exe ../dist/
# Gather transitive DLL deps of the exe AND of SDL2_mixer (music codecs), copy any under /mingw64/bin
deps=$(for f in doomgeneric.exe /mingw64/bin/SDL2_mixer.dll; do ldd "$f"; done | awk '/\/mingw64\/bin/ {print $3}' | sort -u)
cp -v $deps ../dist/
echo "--- dist contents ---"
ls -la ../dist/
