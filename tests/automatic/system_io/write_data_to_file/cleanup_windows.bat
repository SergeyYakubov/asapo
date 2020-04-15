del test_file

icacls test_noaccess /grant:r *S-1-1-0:F
rmdir /S /Q test_noaccess
rmdir /S /Q folder