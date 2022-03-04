---
title: Consumer Clients
---

Consumer client (or consumer) is a part of a distributed streaming system that is responsible for processing streams of data that were created by producer. It is usually a user (beamline scientist, detector developer, physicist, ... ) responsibility to develop a client for specific beamline, detector or experiment using ASAPO Consumer API and ASAPO responsibility to make sure data is delivered to consumers in an efficient and reliable way.

![Docusaurus](/img/consumer-clients.png)

Consumer API is available for C++ and Python and has the following main functionality:

- Create a consumer instance and bind it to a specific beamtime and data source
    - multiple instances can be created (also within a single application) to receive data from different sources
    - an instance id and pipeline step id can be set to allow pipeline monitoring
    - a beamtime token is used for access control
- If needed (mainly for get_next_XX operations), create a consumer group that allows to process messages independently from other groups
- Receive messages  from a specific stream (you can read more [here](data-in-asapo) about data in ASAPO)
    - GetNext to receive process messages one after another without need to know message indexes
        - Consumer API returns a message with index 1, then 2, ... as they were set by producer.
        - This also works in parallel so that payload is distributed within multiple consumers within same consumer group or between threads of a single consumer instance. In parallel case order of indexes of the messages is not determined.
    - GetLast to receive last available message - for e.g. live visualisation
    - GetById - get message by index - provides random access
- Make queries based on metadata contained in a message - returns all messages in a stream with specific metadata. A subset of SQL language is used


All of the above functions can return only metadata part of the message, so that an application can e.g. extract the filename and pass it to a 3rd party tool for processing. Alternative, a function may return the complete message with metadata and data so that consumer can directly process it. An access to the filesystem where data is actually stored is not required in this case.

:::note
In case of dataset family of functions, only list of dataset messages is returned, the data can be retrieved in a separate call.
:::
    
Please refer to [C++](http://asapo.desy.de/cpp/) and [Python](http://asapo.desy.de/python/) documentation for specific details (available from DESY intranet only).



