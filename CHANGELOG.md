## 20.03.1

IMPROVEMENTS
* change behavior when trying to get data from a substream that not exist - return EndOfStream/NoData instead of WrongInput

## 20.03
FEATURES
* introduced substreams for producer/consumer
* introduced timeout for producer requests
* producer accepts "auto" for beamtime, will automatically select a current one for a given beamline
* introduced file transfer service - possibility for consumer clients to receive data also in case filesystem is inaccessible


IMPROVEMENTS
* switch to MongoDB 4.2
* API documentation is available for C++ and Python
* switch to using cmake 3.7+
* error messages in Python as Python strings, not byte objects


BUG FIXES
* consumer operation timout - take duration of the operation into account
* giving warning/error on attempt to send data/metadata with same id