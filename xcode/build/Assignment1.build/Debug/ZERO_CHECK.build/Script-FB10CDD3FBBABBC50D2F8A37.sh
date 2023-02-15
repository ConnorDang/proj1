#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/connordang/Desktop/cs184/hw1/xcode
  make -f /Users/connordang/Desktop/cs184/hw1/xcode/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/connordang/Desktop/cs184/hw1/xcode
  make -f /Users/connordang/Desktop/cs184/hw1/xcode/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/connordang/Desktop/cs184/hw1/xcode
  make -f /Users/connordang/Desktop/cs184/hw1/xcode/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/connordang/Desktop/cs184/hw1/xcode
  make -f /Users/connordang/Desktop/cs184/hw1/xcode/CMakeScripts/ReRunCMake.make
fi

