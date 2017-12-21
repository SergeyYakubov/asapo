mkdir test
for /l %%x in (0, 1, 49) do (
   echo %%x > test\%%x
)

