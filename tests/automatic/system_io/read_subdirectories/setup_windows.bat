mkdir test
mkdir test\subtest1
mkdir test\subtest1\subtest2

mkdir test\subtest3
mkdir test\subtest3\subtest4

ping 1.0.0.0 -n 1 -w 100 > nul

mkdir test_noaccess1
icacls test_noaccess1 /deny users:D



