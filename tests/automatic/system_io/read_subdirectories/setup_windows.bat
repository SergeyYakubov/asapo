mkdir test
mkdir test\subtest1
mkdir test\subtest1\subtest2

mkdir test\subtest3
mkdir test\subtest3\subtest4

ping 192.0.2.1 -n 1 -w 1000 > nul

mkdir test_noaccess1
icacls test_noaccess1 /deny *S-1-1-0:(GA)



