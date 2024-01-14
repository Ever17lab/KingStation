#!/bin/sh

platform=xdk360

mkdir -p ../pkg/msvc/KingStation-360/Debug
mkdir -p ../pkg/msvc/KingStation-360/Release
mkdir -p ../pkg/msvc/KingStation-360/Release_LTCG

for f in *_${platform}.lib ; do
   name=`echo "$f" | sed "s/\(_libks_${platform}\|\).lib$//"`
   echo $name
   cp -f "$f" ../pkg/msvc/KingStation-360/Release_LTCG/libks_${platform}.lib
   cmd.exe /k ${platform}_env.bat ${name}_libks_${platform}
done
