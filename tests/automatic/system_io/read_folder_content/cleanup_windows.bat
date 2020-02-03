rmdir /S /Q test
#icacls test_noaccess1 /grant:r users:D
icacls test_noaccess1 /grant:r *S-1-1-0:F
rmdir /S /Q test_noaccess1
