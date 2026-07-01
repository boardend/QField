# x64 Windows with static libraries but dynamic CRT (/MD for Release, /MDd for Debug)
# This matches the default behavior of most modern CMake projects
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

# This ensures CMAKE_MSVC_RUNTIME_LIBRARY is set to MultiThreaded$<$<CONFIG:Debug>:Debug>DLL
# which means /MD for Release and /MDd for Debug
