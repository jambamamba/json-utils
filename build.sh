#!/bin/bash -xe
set -xe

source share/pins.txt
source share/scripts/helper-functions.sh

function skip(){
    local target="x86"
    parseArgs $@

    if [ "$clean" == "true" ]; then
        return 0
    fi

    local builddir="${target}-build"
    local SHA="$(sudo git config --global --add safe.directory .;sudo git rev-parse --verify --short HEAD)"
    local package="${library}-${SHA}-${target}.tar.xz"

    if [ "$target" == "mingw" ] && \
        [ -f "${builddir}/${library}.dll" ] && \
        [ -f "${builddir}/${package}" ]; then 
        return 1
    elif [ "$target" == "x86" ] && \
        [ -f "${builddir}/lib${library}.so" ] && \
        [ -f "${builddir}/${package}" ]; then 
        return 1
    elif [ "$target" == "arm" ] && \
        [ -f "${builddir}/lib${library}.so" ] && \
        [ -f "${builddir}/${package}" ]; then 
        return 1
    fi
    return 0
}

function build(){
    local clean=""
    local target="x86"
    local cmake_toolchain_file=""
    parseArgs $@
    
    local builddir="${target}-build"
    mkdir -p "${builddir}"

    if [ "$clean" == "true" ]; then
        rm -fr "${builddir}/*"
    fi

    local script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
    export ZLIB_LIBRARY=${script_dir}
    export ZLIB_INCLUDE_DIR=${script_dir}
    local srcdir="$(pwd)"
    pushd "${builddir}"

    local cmake_modules_path="${srcdir}/share/cmake-modules"
    if [ "$target" == "mingw" ]; then
        source "../share/toolchains/x86_64-w64-mingw32.sh"
        cmake \
            -DCMAKE_INSTALL_BINDIR=$(pwd) \
            -DCMAKE_INSTALL_LIBDIR=$(pwd) \
            -DBUILD_SHARED_LIBS=ON \
            -DCMAKE_SKIP_RPATH=TRUE \
            -DCMAKE_SKIP_INSTALL_RPATH=TRUE \
            -DWIN32=TRUE \
            -DMINGW64=${MINGW64} \
            -DWITH_GCRYPT=OFF \
            -DWITH_MBEDTLS=OFF \
            -DHAVE_STRTOULL=1 \
            -DHAVE_COMPILER__FUNCTION__=1 \
            -DHAVE_GETADDRINFO=1 \
            -DENABLE_CUSTOM_COMPILER_FLAGS=OFF \
            -DBUILD_CLAR=OFF \
            -DTHREADSAFE=ON \
            -DCMAKE_SYSTEM_NAME=Windows \
            -DCMAKE_C_COMPILER=$CC \
            -DCMAKE_RC_COMPILER=$RESCOMP \
            -DDLLTOOL=$DLLTOOL \
            -DCMAKE_FIND_ROOT_PATH=/usr/x86_64-w64-mingw32 \
            -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
            -DCMAKE_INSTALL_PREFIX=../install-win \
            -DCMAKE_MODULE_PATH="${cmake_modules_path}" \
            -DCMAKE_PREFIX_PATH="${cmake_modules_path}" \
            -DINSTALL_DEPS="${installdeps}" \
            -G "Ninja" ..
    elif [ "$target" == "arm" ]; then
        # source "${SDK_DIR}/environment-setup-aarch64-fslc-linux"
        source "${SDK_DIR}/environment-setup-cortexa72-oe-linux"
        cmake \
            -DBUILD_SHARED_LIBS=ON \
            -DCMAKE_MODULE_PATH="${cmake_modules_path}" \
            -DCMAKE_PREFIX_PATH="${cmake_modules_path}" \
            -DINSTALL_DEPS="${installdeps}" \
            -G "Ninja" ..
    elif [ "$target" == "x86" ]; then
		export STRIP="$(which strip)"
        cmake \
            -DBUILD_SHARED_LIBS=ON \
            -DCMAKE_MODULE_PATH="${cmake_modules_path}" \
            -DCMAKE_PREFIX_PATH="${cmake_modules_path}" \
            -DINSTALL_DEPS="${installdeps}" \
            -G "Ninja" ..
    fi
    ninja
    popd
}

function installDependencies() {
    local target="x86"
    parseArgs $@
    local builddir="$(pwd)/${target}-build"
    local artifacts_url="/home/$USER/downloads"

    if [ "${installdeps}" == "false" ]; then
        return
    fi

    local libs=(debug_logger)
    for library in "${libs[@]}"; do
        local pin="${library}_pin"
#        echo "${!pin}" #gets the value of variable where the variable name is "${library}_pin"
        local artifacts_file="${library}-${!pin}-${target}.tar.xz"
        installLib $@ library="${library}" artifacts_file="${artifacts_file}" artifacts_url="${artifacts_url}" 
    done
}

function main(){
    local library="json_utils"
    local target="x86"
    local installdeps="true"
    parseArgs $@

    skip $@ library="${library}"
    installDependencies $@ 
    local library="json_utils"
    build $@


    local builddir="/tmp/${library}/${target}-build" # $(mktemp -d)/installs
    copyBuildFilesToInstalls $@ builddir="${builddir}"
    cp -f cJSON/*.h ${builddir}/installs/include/
    compressInstalls $@ builddir="${builddir}" library="${library}"

}

time main $@