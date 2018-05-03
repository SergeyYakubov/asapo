#!/usr/bin/env bash

set -e

res=`$@`

echo $res | grep "\[info\] test info"
echo $res | grep "\[error\] test error"
echo $res | grep "\[debug\] test debug"
echo $res | grep "\[warning\] test warning"
echo $res | grep "test info_mt_0"
echo $res | grep "test info_mt_1"
echo $res | grep "test info_mt_2"
echo $res | grep "test info_mt_3"
echo $res | grep "test_logger"


#error_lev
echo $res | grep "test error_errorlev"
! echo $res | grep "test debug_errorlev"
! echo $res | grep "test info_errorlev"
! echo $res | grep "test warning_errorlev"


#warning_lev
echo $res | grep "test error_warninglev"
! echo $res | grep "test debug_warninglev"
! echo $res | grep "test info_warninglev"
echo $res | grep "test warning_warninglev"


#info_lev
echo $res | grep "test error_infolev"
! echo $res | grep "test debug_infolev"
echo $res | grep "test info_infolev"
echo $res | grep "test warning_infolev"


#none_lev
! echo $res | grep "nonelev"
