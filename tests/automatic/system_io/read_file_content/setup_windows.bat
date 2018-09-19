
mkdir test
echo 123 > test/1
echo unknown_size > test/2
type nul >  file_noaccess
icacls file_noaccess /deny users:D

