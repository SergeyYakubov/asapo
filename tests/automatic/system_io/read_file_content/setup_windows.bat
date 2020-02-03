
mkdir test
echo 123 > test/1
echo unknown_size > test/2
type nul >  file_noaccess
icacls file_noaccess /deny *S-1-1-0:(GA)

