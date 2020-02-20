SET database_name=data
SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"

echo db.dropDatabase() | %mongo_exe% %database_name%
