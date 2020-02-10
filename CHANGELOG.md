+## 20.03 (unreleased)
FEATURES
* introduced substreams for producer/consumer [[JIRA_102](https://agira.desy.de/browse/HIDRA2-102)]

IMPROVEMENTS
* switch to MongoDB 4.2
* receiver use ASAP3 directory structure to save files to
* API documentation is available at [asapo-docs.desy.de](asapo-docs.desy.de)
* switch to using cmake 3.7+

BUG FIXES
* consumer operation timout - take duration of the operation into account