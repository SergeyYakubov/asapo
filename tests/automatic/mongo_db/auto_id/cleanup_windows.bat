SET database_name=data_%%2F%%20%%5C%%2E%%22%%24
SET mongo_exe="c:\Program Files\MongoDB\Server\4.2\bin\mongo.exe"

echo db.dropDatabase() | %mongo_exe% %database_name%
