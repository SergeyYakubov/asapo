#!/usr/bin/env bash

mkdir -p test
echo 123 > test/1

touch  file_noaccess
chmod -rx file_noaccess

