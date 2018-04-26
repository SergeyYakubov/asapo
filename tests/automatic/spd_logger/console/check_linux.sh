#!/usr/bin/env bash

set -e

res=`$@`

echo $res | grep "\[info\] test_info"
echo $res | grep "\[error\] test_error"
echo $res | grep "\[debug\] test_debug"
echo $res | grep "\[warning\] test_warning"
echo $res | grep "test_info_mt_0"
echo $res | grep "test_info_mt_1"
echo $res | grep "test_info_mt_2"
echo $res | grep "test_info_mt_3"
echo $res | grep "test_logger"


#error_lev
echo $res | grep "test_error_errorlev"
! echo $res | grep "test_debug_errorlev"
! echo $res | grep "test_info_errorlev"
! echo $res | grep "test_warning_errorlev"


#warning_lev
echo $res | grep "test_error_warninglev"
! echo $res | grep "test_debug_warninglev"
! echo $res | grep "test_info_warninglev"
echo $res | grep "test_warning_warninglev"


#info_lev
echo $res | grep "test_error_infolev"
! echo $res | grep "test_debug_infolev"
echo $res | grep "test_info_infolev"
echo $res | grep "test_warning_infolev"


#none_lev
! echo $res | grep "nonelev"
