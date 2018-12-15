#!/bin/bash
#
# Script to kickstart creating a new sample directory, so you don't forget to include necessary files especially .gitignore.
# ex: bash createsample.sh <sample-name> <main-source-name> <sample-based-name>

# check if user inputs sample-name
if [ "$#" -lt 2 ]; then
  echo "Usage: bash createsample.sh <sample-name> <main-source-name> <sample-based-name>"
  exit -1
fi

SAMPLE_DIR=$1
SAMPLE_BASED=$3
MAIN_SRC=$2

# make a new directory
mkdir $SAMPLE_DIR

# copy all files from sample based to kickstart
# not to copy any hidden files of this directory, but copy both files and directories inside
cp -a $SAMPLE_BASED/* $SAMPLE_DIR/
# clean the destination sample dir
cd $SAMPLE_DIR && make clean && cd ..

# copy .gitignore file
cp -p template.gitignore $SAMPLE_DIR/.gitignore
# copy main source file
cp -p template.c $SAMPLE_DIR/$MAIN_SRC.c

# get PROGRAM file name of sample based
BASED_PROGRAM=$(awk -F'=' '$1 == "PROGRAM" {print $2}' $SAMPLE_DIR/Makefile)

# modify destination Makefile's PROGRAM and OUTPUT to be correct name
# change entire line matching the desire text to value that suitable for project to be created
# note: use gnu sed as macOS version will create backup file for us without a way to disable.
gsed -i "1s/PROGRAM\=.*/PROGRAM\=$MAIN_SRC/" $SAMPLE_DIR/Makefile
gsed -i "2s/OUTPUT\=.*/OUTPUT\=$MAIN_SRC/" $SAMPLE_DIR/Makefile

# shout out hint to decide whether to remove existing main source from sample based dir, or completely use content from template.c
echo ":: Now decide whether to use '$MAIN_SRC.c' or existing one '$BASED_PROGRAM.c'."
