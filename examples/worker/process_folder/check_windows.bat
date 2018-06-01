mkdir test
echo "" > test/1

.\worker_processfolder test | findstr /c:"Processed 1 file(s)"

rmdir /S /Q test
