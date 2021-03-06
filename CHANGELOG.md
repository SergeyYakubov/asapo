## 21.12.0

FEATURES
* Consumer API: Get last within consumer group returns message only once
* Producer API: An option to write raw data to core filesystem directly
* Consumer/Producer API - packages for Debian 11.1, wheel for Python 3.9
* Consumer/Producer API - dropped Python 2 support for wheels and packages for new Debian/CentOS versions

INTERNAL
* Improved logging - tags for beamline, beamtime, ...
* Updated orchestration tools to latest version

## 21.09.0

FEATURES
* Producer API: C client
* Introduce a token to send data in "raw" mode without LDAP authorization

IMPROVEMENTS
* Allow using ASAPO for commissioning beamtimes
* Implement token revocation
* Updated website

BUG FIXES
* Consumer/Producer API: fixed bug with "_" in stream name

INTERNAL
* Improved authoriation service caching
* Added profiling for Go services
* Added metrics and alerts for asapo services

## 21.06.0

FEATURES
* Consumer API: C client
* Producer API: An option to automatically generate message id (use sparingly, reduced performance possible)

IMPROVEMENTS
* Consumer/Producer API - allow any characters in source/stream/group names
* Consumer/Producer API - introduce stream metadata
* Consumer API - an option to auto discovery of data folder when consumer client uses file transfer service (has_filesystem=False)
* Improved build procedure - shared libaries, added pkg-config and cmake config for asapo clients

BUG FIXES
* Consumer API: multiple consumers from same group receive stream finished error
* Consumer API: return ServiceUnavailable instead of Unauthorized in case an authorization service is unreachable

## 21.03.3

BUG FIXES
* Consumer API: fix return error type when sending acknowledgement second time
* Producer API: fix GetStreamInfo/stream_info and GetLastStream/last_stream for datasets

## 21.03.2

FEATURES
* implemented possibility to delete stream (only metadata, not files yet)

IMPROVEMENTS
* Consumer API - retry file delivery/reading with timeout (can be useful for the case file arrives after was metadta ingested, e.g. for slow NFS transfer,...)

BUG FIXES
* Consumer API: fix race condition in GetStreamList/get_stream_list
* Producer API: fix segfault in send_stream_finished_flag
* Producer API: fix deadlock in producer timeout

## 21.03.1

BUG FIXES
* Core services: fix LDAP authorization for raw data type Producers

## 21.03.0

 IMPROVEMENTS
* Producer API - queue limits in Python, for C++ return original data in error custom data
* Consumer API - add GetCurrentDatasetCount/get_current_dataset_count function with option to include or exclude incomplete datasets
* Consumer API - GetStreamList/get_stream_list - can filter finished/unfinished streams now
* Producer/Consumer API - StreamInfo structure/Python dictionary include more information (is stream finished or not, ...)
* Switch to JWT tokens (token has more symbols, expiration time, can be revoked and there are two type of tokens - with read/write access rights)
* Improved versioning. Producer/Consumer API - introduce GetVersionInfo/get_version_info, compatiblity check between clients and server

BREAKING CHANGES
* Consumer API (C++ only)- GetStreamList has now extra argument StreamFilter
* Consumer/Producer libraries need to be updated due to protocol changes

## 20.12.0

FEATURES
* implemented possibility to send data without writing to database (no need of consecutive indexes, etc. but will not be able to consume such data)
* allow to return incomplete datasets (wihout error if one sets minimum dataset size, otherwise with "partial data" error)

 IMPROVEMENTS
* Consumer API - change behavior of GetLast/get_last - do not change current pointer after call
* Consumer API - add interrupt_current_operation to allow interrupting (from a separate thread) long consumer operation
* Producer API - return original data in callback payload.
* Producer API - allow to set queue limits (number of pending requests and/or max memory), reject new requests if reached the limits
* building rpm, deb and exe packages for client libs

BREAKING CHANGES
* Consumer API - get_next_dataset, get_last_dataset, get_dataset_by_id return dictionary with 'id','expected_size','content' fields, not tuple (id,content) as before
* Consumer API - remove group_id argument from get_last/get_by_id/get_last_dataset/get_dataset_by_id functions
* Producer API - changed meaning of subsets (subset_id replaced with dataset_substream and this means now id of the image within a subset (e.g. module number for multi-module detector)), message_id is now a global id of a multi-set data (i.g. multi-image id)
    ####  renaming - general
* stream -> data_source, substream -> stream
* use millisecond everywhere for timeout/delay
* use term `message` for blob of information we send around, rename related structs, parameters, ...
* C++ - get rid of duplicate functions with default stream
    ####  renaming - Producer API
* SendData/send_data -> Send/send
* SendXX/send_xx -> swap parameters (stream to the end)
* id_in_subset -> dataset_substream
* subset_size -> dataset_size (and in general replace subset with dataset)
    ####  renaming - Consumer API
* broker -> consumer
* SetLastReadMarker/set_lastread_marker -> swap arguments
* GetUnacknowledgedTupleIds/get_unacknowledged_tuple_ids -> GetUnacknowledgedMessages/get_unacknowledged_messages
* GetLastAcknowledgedTulpeId/get_last_acknowledged_tuple_id -> GetLastAcknowledgedMessage/get_last_acknowledged_message
* GetUnacknowledgedMessages, -> swap parameters (stream to the end)

BUG FIXES
* fix memory leak bug in Python consumer library (lead to problems when creating many consumer instances)


## 20.09.1

FEATURES
* New function GetLastStream/last_stream in Producer API - returns info for a stream which was created last

IMPROVEMENTS
* Each message automatically gets a timestamp (nanoseconds from Linux epoch) at the moment it is being inserted to a database
* GetStreamList/get_stream_list returns now sorted (by timestamp of the earliest message) list of streams. Parameter `from` allows to limit the list

BREAKING CHANGES
* GetStreamList/get_stream_list returns now not an array of strings, but array of StreamInfos/dictionaries

## 20.09.0

FEATURES
* implemented negative acknowledges and data redelivery - data will be redelivered automatically for get_next calls if it is not acknowledged during a given period or a consumer sent a negative acknowledge
* introduced data source types - "raw" data is written to beamline filesystem and this can only be done from a certain IPs (detector PC,..),
"processed" data is written to core filesystem. File paths must now start with  `raw/`  or  `processed/`
* Added RDMA support for the communication between consumer and receiver. (Improves transfer speeds while using less CPU resources)
  Requires LibFabric v1.11.0
  Receiver must have network mode 'Fabric' enabled and RDMAable AdvertiseURI. See config `DataServer.{AdvertiseURI, NetworkMode}`
* Added 'ASAPO_PRINT_FALLBACK_REASON' as an environment variable for the consumer in order to get a message why TCP was used
* Added new consumer broker API call 'ForceNoRdma' to always use TCP and ignore any RDMA capabilities
* Added new consumer broker API call 'CurrentConnectionType' to see what connection type is currently used

BUG FIXES
* fix data query images when beamtime_id starts with number

BREAKING CHANGES
* an extra parameter in producer constructor for data source type
* path of the files that are send from producer to asapo must start with `raw/` for raw source type or `processed/` for processed source type, otherwise the files will not be written and an error will be sent back

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
* implemented acknowledeges - one can acknowledge a message, get last acknowledged tuple id, get list of unacknowledged tuple ids
* implement getting stream info (contains last id) by producer client (not need to have consumer client)

IMPROVEMENTS
* change behavior when trying to get data from a stream that does not exist - return EndOfStream instead of WrongInput
* change behavior of GetLastXX/get_lastXX functions - current pointer is not being set to the end of a stream after this command anymore
* stream name added to producer callback output for Python
* added simple C++ examples

BUG FIXES
* check message ids should be positive

## 20.03.0
FEATURES
* introduced streams for producer/consumer
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
