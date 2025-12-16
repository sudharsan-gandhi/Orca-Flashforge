
set(_context_abi_line "")
set(_context_arch_line "")
if (APPLE AND CMAKE_OSX_ARCHITECTURES)
    if (CMAKE_OSX_ARCHITECTURES MATCHES "x86")
        set(_context_abi_line "-DBOOST_CONTEXT_ABI:STRING=sysv")
    elseif (CMAKE_OSX_ARCHITECTURES MATCHES "arm")
        set (_context_abi_line "-DBOOST_CONTEXT_ABI:STRING=aapcs")
    endif ()
    set(_context_arch_line "-DBOOST_CONTEXT_ARCHITECTURE:STRING=${CMAKE_OSX_ARCHITECTURES}")
endif ()

orcaslicer_add_cmake_project(Boost
    URL "https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.gz"
    URL_HASH SHA256=e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
    LIST_SEPARATOR |
    CMAKE_ARGS
        -DBOOST_EXCLUDE_LIBRARIES:STRING=contract|fiber|numpy|stacktrace|wave|test
        -DBOOST_LOCALE_ENABLE_ICU:BOOL=OFF # do not link to libicu, breaks compatibility between distros
        -DBUILD_TESTING:BOOL=OFF
        "${_context_abi_line}"
        "${_context_arch_line}"
)

set(DEP_Boost_DEPENDS ZLIB)