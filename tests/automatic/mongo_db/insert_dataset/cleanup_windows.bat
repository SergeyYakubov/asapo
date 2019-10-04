SET database_name=data
SET mongo_exe="c:\Program Files\MongoDB\Server\3.6\bin\mongo.exe"

echo db.dropDatabase() | %mongo_exe% %database_name%
