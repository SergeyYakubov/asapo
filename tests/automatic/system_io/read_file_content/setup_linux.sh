#!/usr/bin/env bash

mkdir -p test
echo 123 > test/1
echo unknown_size > test/2

touch  file_noaccess
chmod -rx file_noaccess

