##20.09.0

FEATURES
* implemented data resend - data will be redelivered if it is not acknowledged during a given period or a consumer sent a negative acknowledge
* Added RDMA support for the communication between consumer and receiver. (Improves transfer speeds while using less CPU resources)
  Requires LibFabric v1.11.0
  Receiver must have network mode 'Fabric' enabled and RDMAable AdvertiseURI. See config `DataServer.{AdvertiseURI, NetworkMode}`
* Added 'ASAPO_PRINT_FALLBACK_REASON' as an environment variable for the consumer in order to get a message why TCP was used
* Added new consumer broker API call 'ForceNoRdma' to always use TCP and ignore any RDMA capabilities
* Added new consumer broker API call 'CurrentConnectionType' to see what connection type is currently used

BUG FIXES
* fix data query images when beamtime_id starts with number 

## 20.06.3

BUG FIXES
* fix retrieve_data in Python modules for data ingested using metadata only mode
* fix asapo orchestration image stabilize nginx and update fluentd configuration to follow Nomad jobs log rotation 

## 20.06.2

BUG FIXES
* file size obtained automatically when retrieving remote data ingested using metadata only mode

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
