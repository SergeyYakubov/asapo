rmdir /S /Q test
icacls file_noaccess /grant:r *S-1-1-0:F
del file_noaccess
