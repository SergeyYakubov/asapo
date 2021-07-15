---
title: Core Architecture
---

For those who are curious about ASAPO architecture, the diagram shows some details. Here arrows with numbers is an example of data workflow explained below.

![Docusaurus](/img/core-architecture.png)

## Data Workflow (example)
the workflow can be split into two more or less independent tasks - data ingestion and data retrieval

### Data ingestion (numbers with i on the diagram)
1i) As we [know](producer-clients.md), producer client is responsible for ingesting data in the system. Therefore the first step is to detect that the new message is available. This can be done using another tool developed at DESY named [HiDRA](https://confluence.desy.de/display/FSEC/HiDRA). This tool monitors the source of data (e.g. by monitoring a filesystem or using HTTP request or ZeroMQ streams, depending on detector type)

2i) HiDRA (or other user application) then uses ASAPO Producer API to send messages (M1 and M2 in our case) in parallel to ASAPO Receiver. TCP/IP or RDMA protocols are used to send data most efficiently. ASAPO Receiver receives data in a memory cache

3i) - 4i) ASAPO saves data to a filesystem and adds a metadata record to a database

5i) A feedback is send to the producer client with success or error message (in case of error, some of the step above may not happen)

### Data retrieval (numbers with r on the diagram)

[Consumer client](consumer-clients.md)) is usually a user application that retrieves data from the system to analyse/process it.

The first step to retrieve a message via Consumer API is to pass the request to the Data Broker (1r). The Data Broker retrieves the metadata information about the message from the database (2r) and returns it to the Consumer Client. The Consumer Client analyses the metadata information and decides how to get the data. It the data is still in the  Receiver memory cache, the client requests data from there via a Data Server (which is a part of ASAPO Receiver). Otherwise, client gets the data from the filesystem - directly if the filesystem is accessible on the machine where the client is running or via File Transfer Service if not.




