## 20.06.1

IMPROVEMENTS
* allow arbitrary group id

## 20.06.0
FEATURES
* implemented acknowledeges - one can acknowledge a data tuple, get last acknowledged tuple id, get list of unacknowledged tuple ids
* implement getting substream info (contains last id) by producer client (not need to have consumer client)

IMPROVEMENTS
* change behavior when trying to get data from a substream that does not exist - return EndOfStream instead of WrongInput
* change behavior of GetLastXX/get_lastXX functions - current pointer is not being set to the end of a substream after this command anymore
* substream name added to producer callback output for Python
* added simple C++ examples

BUG FIXES
* check data tuple ids should be positive

## 20.03.0
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