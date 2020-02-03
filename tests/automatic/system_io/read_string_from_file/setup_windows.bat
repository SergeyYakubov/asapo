
mkdir test
echo "123" > test\1


type nul >  file_noaccess
icacls file_noaccess /deny *S-1-1-0:(GA)

