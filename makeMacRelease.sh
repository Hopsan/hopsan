#!/bin/sh                                                                                                                                                                    
#                                                                                                                                                                            
# @file   makeMacRelease.sh                                                                                                                                                  
# @author Magnus Sethson <magnus.sethson@liu.se>                                                                                                                             
# @date   2015-02-27                                                                                                                                                         
#                                                                                                                                                                            
# @brief Script for building, configuring, compiling and adjusting HopsanGUI for Mac OS X
#                                                                                                                                                                            
#

rm -fr bin/*

# Building dependencies

cd dependencies

. setupQwt.sh --force
. setupFMILibrary.sh --force

cd ..


# Building for RELEASE

. ./packaging/mac-app/build.sh --release

echo RELEASE built

# Building for DEBUG

. ./packaging/mac-app/build.sh --debug

echo DEBUG built

du -sh ./bin/HopsanGUI.app
du -sh ./bin/HopsanGUI_d.app
