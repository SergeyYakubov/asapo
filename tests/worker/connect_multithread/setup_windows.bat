mkdir test
for /l %%x in (1, 1, 10) do (
   type nul > test\%%x
)

