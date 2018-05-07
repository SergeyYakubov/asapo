#rmdir /S /Q test
icacls test_noaccess1 /grant:r users:D
rmdir /S /Q test_noaccess1
