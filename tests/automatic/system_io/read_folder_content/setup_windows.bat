mkdir test
mkdir test\subtest
mkdir test\subtest\subtest2

type nul > test\2
ping 192.0.2.1 -n 1 -w 1000 > nul
type nul > test\3
ping 192.0.2.1 -n 1 -w 1000 > nul
type nul > test\subtest\subtest2\4
ping 192.0.2.1 -n 1 -w 1000 > nul
echo | set /p dummyName="1234" > test\1

mkdir test_noaccess1
#icacls test_noaccess1 /deny users:D
icacls test_noaccess1 /deny *S-1-1-0:(GA)



