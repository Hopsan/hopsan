#!/bin/bash
# Shell script for installing the intended files from a pre-build hopsan root directory

function install_dynamic_libs_if_exist {
  local src_dir="$1"
  local libname=$2
  local dst_dir="$3"
  if [[ "$OSTYPE" == "darwin"* ]]; then
    local lib_suffix=.dylib
  else
    local lib_suffix=.so
  fi

  if [[ -d "${dst_dir}" ]]; then
      if [[ -d "${src_dir}" ]]; then
          # TODO find files
          cp -a "${src_dir}"/${libname}${lib_suffix}* "${dst_dir}"
          chmod u=rw,go=r "${dst_dir}/"${libname}${lib_suffix}*
      fi
  else
    echo Error: Destination directory ${dst_dir} does not exist
  fi
}

function install_dir {
    src_dir="$1"
    dst_dir="$2"
    mkdir -p "${dst_dir}"
    cp -a "${src_dir}" "${dst_dir}"
    find "${dst_dir}" -name ".git*" | xargs -r rm -r
    chmod -R u+rwX,go-w+rX "${dst_dir}"
}

E_BADARGS=65
if [ $# -lt 2 ]; then
  echo "Error: To few input arguments!"
  echo "Usage: `basename $0` {srcDir dstDir}"
  exit $E_BADARGS
fi

srcDir=${1%/}
dstDir=${2%/}
echo "Installing Hopsan from $srcDir to $dstDir"

# Copy whole directories
# ======================
install_dir  $srcDir/hopsanc/include                           $dstDir/hopsanc

install_dir  $srcDir/HopsanCore/include                        $dstDir/HopsanCore
install_dir  $srcDir/HopsanCore/src                            $dstDir/HopsanCore
install_dir  $srcDir/HopsanCore/dependencies                   $dstDir/HopsanCore
rm -rf $dstDir/HopsanCore/dependencies/sundials/config
rm -rf $dstDir/HopsanCore/dependencies/sundials/doc
rm -rf $dstDir/HopsanCore/dependencies/sundials/examples
rm -rf $dstDir/HopsanCore/dependencies/sundials/test
rm -rf $dstDir/HopsanCore/dependencies/sundials/*.pdf
rm -rf $dstDir/HopsanCore/dependencies/sundials/*.txt

install_dir  $srcDir/componentLibraries/defaultLibrary         $dstDir/componentLibraries
install_dir  $srcDir/componentLibraries/exampleComponentLib    $dstDir/componentLibraries
install_dir  $srcDir/componentLibraries/ModelicaExampleLib     $dstDir/componentLibraries
install_dir  $srcDir/componentLibraries/extensionLibrary       $dstDir/componentLibraries
install_dir  $srcDir/componentLibraries/autoLibs               $dstDir/componentLibraries

install_dir  $srcDir/Models/Example\ Models                    $dstDir/Models
install_dir  $srcDir/Models/Component\ Test                    $dstDir/Models

install_dir  $srcDir/doc/html                                  $dstDir/doc

install_dir  $srcDir/Scripts                                   $dstDir

# Copy compiled libs and exec files
# =================================
install_dir  $srcDir/bin                                       $dstDir

# Copy dependencies files
# =======================
srcDeps=${srcDir}/dependencies

install_dir  ${srcDeps}/katex                                  $dstDir/dependencies
install_dir  ${srcDeps}/fmilibrary                             $dstDir/dependencies
install_dir  ${srcDeps}/fmi4c                                  $dstDir/dependencies
install_dir  ${srcDeps}/xerces                                 $dstDir/dependencies
install_dir  ${srcDeps}/libzip                                 $dstDir/dependencies

install_dynamic_libs_if_exist  ${srcDeps}/qwt/lib         libqwt            $dstDir/bin
install_dynamic_libs_if_exist  ${srcDeps}/zeromq/lib      libzmq            $dstDir/bin
install_dynamic_libs_if_exist  ${srcDeps}/fmilibrary/lib  libfmilib_shared  $dstDir/bin
install_dynamic_libs_if_exist  ${srcDeps}/discount/lib    libmarkdown       $dstDir/bin
install_dynamic_libs_if_exist  ${srcDeps}/hdf5/lib        libhdf5           $dstDir/bin
install_dynamic_libs_if_exist  ${srcDeps}/hdf5/lib        libhdf5_cpp       $dstDir/bin
install_dynamic_libs_if_exist  ${srcDeps}/xerces/lib      libxerces-c       $dstDir/bin
install_dynamic_libs_if_exist  ${srcDeps}/xerces/lib      libxerces-c-3.2   $dstDir/bin
install_dynamic_libs_if_exist  ${srcDeps}/libzip/lib      libzip            $dstDir/bin

# Install additional files
# =====================
install -m644 -t $dstDir                                   $srcDir/hopsan-default-configuration.xml
install -m644 -t $dstDir                                   $srcDir/Hopsan-release-notes.txt
install -m644 -t $dstDir                                   $srcDir/README.md

# Strip any runpaths to dependencies directory
# from ELF binaries. Note! ($ORIGIN/./) will remain.
# By first moving the source dependencies directory
# the runpaths will no longer be valid and patchelf
# will remove them.
# ==================================================
if [[ $(command -v patchelf) ]]; then
  mv ${srcDeps} ${srcDeps}_temporary
  find ${dstDir}/bin -type f -executable -exec patchelf --shrink-rpath {} \;
  mv ${srcDeps}_temporary ${srcDeps}
fi
