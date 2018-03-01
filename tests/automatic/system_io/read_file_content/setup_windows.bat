
mkdir test
echo 123 > test/1
type nul >  file_noaccess
icacls file_noaccess /deny users:D

