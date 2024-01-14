#!/bin/sh

platform=xdk1

ip='192.168.178.227'

mkdir -p ../pkg/msvc/KingStation-Xbox1/Debug
mkdir -p ../pkg/msvc/KingStation-Xbox1/Release
mkdir -p ../pkg/msvc/KingStation-Xbox1/Release_LTCG

for f in *_${platform}.lib ; do
   name=`echo "$f" | sed "s/\(_libks_${platform}\|\).lib$//"`
   echo $name
   if [ $name = "tyrquake" ] || [ $name = "genesis_plus_gx" ] ; then
      echo "Applying whole archive linking for this core..."
   	cp -f "$f" ../pkg/msvc/KingStation-Xbox1/Release_LTCG_BigStack/libks_${platform}.lib
   	cmd.exe /k ${platform}_env_bigstack.bat ${name}_libks_${platform}
   else
   	cp -f "$f" ../pkg/msvc/KingStation-Xbox1/Release_LTCG/libks_${platform}.lib
   	cmd.exe /k ${platform}_env.bat ${name}_libks_${platform}
   fi
done
