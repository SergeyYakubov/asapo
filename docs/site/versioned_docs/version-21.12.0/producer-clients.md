---
title: Producer Clients
---

Producer client (or producer) is a part of a distributed streaming system that is responsible for creating data streams (i.e. ingesting data in the system). It is usually a user (beamline scientist, detector developer, physicist, ... ) responsibility to develop a client for specific beamline, detector or experiment using ASAPO Producer API and ASAPO responsibility to make sure data is transferred and saved an in efficient and reliable way.

![Docusaurus](/img/producer-clients.png)

Producer API is available for C++ and Python and has the following main functionality:

- Create a producer instance and bind it to a specific beamtime and data source
multiple instances can be created (also within a single application) to send data from different sources
- Send messages to a specific stream (you can read more [here](data-in-asapo) about data in ASAPO)
    - each message must have a consecutive integer index, ASAPO does not create indexes automatically
    - to compose datasets, dataset substream (and dataset size) should be send along with each message
    - messages are sent asynchronously, in parallel using multiple threads 
    - retransfer will be attempted in case of system failure
    - a callback function can be provided to react after data was sent/process error
    
Please refer to [C++](http://asapo.desy.de/cpp/) and [Python](http://asapo.desy.de/python/) documentation for specific details (available from DESY intranet only).



