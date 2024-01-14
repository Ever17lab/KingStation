#!/bin/sh
# app stuff

rm -rf KingStation.app

mkdir -p KingStation.app/Contents/MacOS
cp -r pkg/apple/OSX/* KingStation.app/Contents
cp KingStation KingStation.app/Contents/MacOS

mv KingStation.app/Contents/Info_Metal.plist KingStation.app/Contents/Info.plist

sed -i'.bak' 's/\${EXECUTABLE_NAME}/KingStation/' KingStation.app/Contents/Info.plist
sed -i'.bak' 's/\$(PRODUCT_BUNDLE_IDENTIFIER)/com.libks.KingStation/' KingStation.app/Contents/Info.plist
sed -i'.bak' 's/\${PRODUCT_NAME}/KingStation/' KingStation.app/Contents/Info.plist
sed -i'.bak' 's/\${MACOSX_DEPLOYMENT_TARGET}/10.13/' KingStation.app/Contents/Info.plist

cp media/KingStation.icns KingStation.app/Contents/Resources/

# dmg stuff

umount wc
rm -rf KingStation.dmg wc empty.dmg

mkdir -p template
hdiutil create -fs HFSX -layout SPUD -size 200m empty.dmg -srcfolder template -format UDRW -volname KingStation -quiet
rmdir template

mkdir -p wc
hdiutil attach empty.dmg -noautoopen -quiet -mountpoint wc
rm -rf wc/KingStation.app
ditto -rsrc KingStation.app wc/KingStation.app
ln -s /Applications wc/Applications
WC_DEV=`hdiutil info | grep wc | grep "Apple_HFS" | awk '{print $1}'` && hdiutil detach $WC_DEV -quiet -force
hdiutil convert empty.dmg -quiet -format UDZO -imagekey zlib-level=9 -o KingStation.dmg

umount wc
rm -rf wc empty.dmg
