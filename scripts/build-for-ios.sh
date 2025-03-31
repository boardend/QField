#!/bin/bash

set -euo pipefail

# cmake -S . -B build-arm64-ios \
# 	-DVCPKG_HOST_TRIPLET=x64-osx \
# 	-DVCPKG_TARGET_TRIPLET=arm64-ios \
# 	-DWITH_VCPKG=ON \
#	-DWITH_SPIX=OFF \
#	-DWITH_BLUETOOTH=OFF \
#	-DWITH_SERIALPORT=OFF \
# 	-DVCPKG_BUILD_TYPE=release \
# 	-DCMAKE_SYSTEM_NAME=iOS \
# 	-DCMAKE_OSX_SYSROOT=iphoneos \
# 	-DCMAKE_OSX_ARCHITECTURES=arm64 \
# 	-DCMAKE_SYSTEM_PROCESSOR=aarch64 \
# 	-GXcode

# cmake --build build-arm64-ios

cmake -S . -B build-x64-ios \
	-DVCPKG_HOST_TRIPLET=x64-osx \
	-DVCPKG_TARGET_TRIPLET=x64-ios \
	-DWITH_VCPKG=ON \
	-DWITH_SPIX=OFF \
	-DWITH_BLUETOOTH=OFF \
	-DWITH_SERIALPORT=OFF \
	-DCMAKE_SYSTEM_NAME=iOS \
	-DCMAKE_OSX_SYSROOT=iphonesimulator \
	-DCMAKE_SYSTEM_PROCESSOR=x86_64 \
	-DCMAKE_OSX_ARCHITECTURES=x86_64 \
	-GXcode

cmake --build build-x64-ios
