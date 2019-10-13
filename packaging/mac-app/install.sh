#!/bin/sh
#
# @file   install.sh
# @author Magnus Sethson <magnus.sethson@liu.se>
# @date   2015-02-27
#
# @brief Instalation script template for Mac OS X
#
#$Id$
# 

APPDIR=HopsanGUI.app

mkdir -p $HOME/Library/Preferences/Hopsan/
mkdir -p $HOME/Library/Application Support/HopsanGUI
mkdir -p $HOME/Documents/Hopsan
mkdir -p $HOME/Documents/Hopsan/Backup
mkdir -p $HOME/Documents/Hopsan/Models
mkdir -p $HOME/Documents/Hopsan/Scripts
mkdir -p $HOME/Documents/Hopsan/import/FMU

cp -prf $APPDIR/Contents/Resources/hopsan-default-configuration.xml $HOME/Library/Preferences/Hopsan/
