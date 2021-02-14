#!/bin/bash
TCR=$(tput cr)
TNRM=$(tput sgr0)
TRED=$(tput setaf 1)
TGRN=$(tput setaf 2)
TYEL=$(tput setaf 3)
TBLU=$(tput setaf 4)
TMAG=$(tput setaf 5)
TCYN=$(tput setaf 6)

TESTVOLUME="TestVolume"
gigabytes=2


function createCaseSensitiveTestVolume()
{
   echo "${TBLU}Creating volume mounted as ${TNRM} /Volumes/${TESTVOLUME} ..."
   if [ -d "/Volumes/${TESTVOLUME}" ]; then
      echo "${TYEL}WARNING${TNRM}: Volume already exists: /Volumes/${TESTVOLUME}"
      return
   fi

   echo "image is ${TESTVOLUME}.sparseimage"
   if [ -f "${TESTVOLUME}.sparseimage" ]; then
      echo "${TRED}WARNING:${TNRM}"
      echo "         File already exists: ${TESTVOLUME}.sparseimage ${TNRM}"
      echo "         This file will be mounted as /Volumes/${TESTVOLUME} ${TNRM}"

      # Give a couple of seconds for the user to react
      sleep 3

   else

      hdiutil create "${TESTVOLUME}"               \
                      -volname "${TESTVOLUME}"     \
                      -type SPARSE                 \
                      -size ${gigabytes}g          \
                      -stretch ${gigabytes}g       \
                      -fs HFSX                     \
                      -quiet                       \
                      -noautostretch               \
                      -puppetstrings
   fi

   hdiutil mount "${TESTVOLUME}.sparseimage"
   gcc -o zerofile zerofile.c
   cp zerofile "/Volumes/${TESTVOLUME}"
}

function realClean( )
{
   # Eject the disk instead of unmounting it or you will have
   # a lot of disks hanging around.
   if [ -d  "/Volumes/${TESTVOLUME}" ]; then
      echo "${TBLU}Ejecting  /Volumes/${TESTVOLUME} ${TNRM}"
      hdiutil eject "/Volumes/${TESTVOLUME}"
   fi

   if [ -f  "${TESTVOLUME}.sparseimage" ]; then
      echo "${TBLU}Removing ${TESTVOLUME}.sparseimage ${TNRM}"
      rm "${TESTVOLUME}.sparseimage"
   fi
}

function showHelp( )
{
cat <<'HELP_EOF'
   This shell script is a front end setup script to test zerofile. Sorry, testing is on Mac only.

   Options:
      -s  = Setup by creating a mounted volume.
      -c  = Clean by unmounting test volume.
      -g  = Gigabytes of test volume to create. Default is 2.
      -h  = Help, this limited help.

HELP_EOF
}

if [[ "$OSTYPE" != "darwin"* ]]; then
   echo "Sorry, can only test on a mac since it has the ability to create temporary volumes for testing"
   exit 666
fi

while getopts "h?scg:" opt; do
   case ${opt} in
      h)
          showHelp
          exit 0
          ;;
          #####################
      s)
         createCaseSensitiveTestVolume
         exit 0
         ;;
      c)
         realClean
         exit 0
         ;;
      g)
         gigabytes=${OPTARG}
         ;;
      \?)
          exit 666
          ;;
          #####################
      :)
          echo "${TRED}Option ${TNRM}-${OPTARG} requires an argument."
          exit 666
          ;;
          #####################
   esac
done




