+## 20.03 (unreleased)
FEATURES
* introduced substreams for producer/consumer
* introduced timeout for producer requests

IMPROVEMENTS
* switch to MongoDB 4.2
* receiver use ASAP3 directory structure to save files to
* API documentation is available at
* switch to using cmake 3.7+
* error messages in Python as Python strings, not byte objects

BUG FIXES
* consumer operation timout - take duration of the operation into account
* giving warning/error on attempt to send data/metadata with same id