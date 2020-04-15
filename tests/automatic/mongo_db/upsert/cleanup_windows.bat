SET database_name=test
SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"

echo db.dropDatabase() | %mongo_exe% %database_name%
