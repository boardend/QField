#!/bin/bash

set -euo pipefail

export ANDROID_SDK_ROOT=$HOME/Android/Sdk
export ANDROID_NDK_VERSION=26.3.11579264
export ANDROID_BUILD_TOOLS_VERSION=35.0.0
export ANDROID_NDK_HOME=$HOME/Android/Sdk/ndk/${ANDROID_NDK_VERSION}

export JAVA_HOME=$HOME/dev/qfield-dev/android-studio/jbr
export PATH=$JAVA_HOME/bin:$PATH

export APP_VERSION=1.0.0
export APK_VERSION_CODE=1
export APP_VERSION_STR=1.0.0
export APP_PACKAGE_NAME=qfieldembedded
export APP_ICON=qfield_logo
export APP_NAME="QField Embedded"

mkdir -p build-x64-android

cmake \
    -S . -B $(git rev-parse --show-toplevel)/build-x64-android \
    -GNinja \
    -D WITH_VCPKG=ON \
    -D VCPKG_TARGET_TRIPLET=x64-android \
    -D SYSTEM_QT=ON \
    -D CMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -D ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT}" \
    -D ANDROID_NDK_VERSION="${ANDROID_NDK_VERSION}" \
    -D ANDROID_BUILD_TOOLS_VERSION="${ANDROID_BUILD_TOOLS_VERSION}" \
    -D VCPKG_INSTALL_OPTIONS="--allow-unsupported" \
    -D WITH_ALL_FILES_ACCESS="OFF" \
    -D WITH_SPIX=OFF \
    -D APP_VERSION="${APP_VERSION}" \
    -D APK_VERSION_CODE="${APK_VERSION_CODE}" \
    -D APP_VERSION_STR="${APP_VERSION_STR}" \
    -D APP_PACKAGE_NAME="${APP_PACKAGE_NAME}" \
    -D APP_ICON="${APP_ICON}" \
    -D APP_NAME="${APP_NAME}"

cmake --build $(git rev-parse --show-toplevel)/build-x64-android --target aar