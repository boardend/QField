# ARM64 Windows with static libraries but dynamic CRT (/MD for Release, /MDd for Debug)
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
