---
title: Overview
---



ASAP::O (or ASAPO) is a high performance distributed streaming platform. It is being developed at DESY and is mainly aimed to support online/offline analysis of experimental data produced at its facilities. The ideas behind are quite similar to that of Apache Kafka and similar messaging solutions, but ASAPO is developed and tuned for scientific use cases with their specific workflows and where the size of the messages is much large (MBs to GBs as compared to KBs in traditional systems).



ASAPO has the following key capabilities:

- Deliver data produced by an experimental facility (e.g. detector) to a data center in a high-performant fault-tolerant way
- Consume this data in various modes (as soon as new data occurs, random access, latest available data, in parallel, ...)
- Ingest own data/ create computational pipelines


ASAPO consists of the following three components:

- Core services (run in background on a single node or a cluster and provide ASAPO functionality)
- Producer API to ingest data into the system
- Consumer API to retrieve data from the system

### Bird's eye view

A workflow when using ASAPO can be represented as follows:

![Docusaurus](/img/asapo_bird_eye.png)
        

Usually, an end user can see ASAPO core services as a black box. But some more details are given [here](core-architecture).

Next, one can learn more about following concepts:

- [Data in ASAPO](data-in-asapo)
- [Producer clients](producer-clients)
- [Consumer clients](consumer-clients)

You can also compare with other solutions, jump directly to [Getting Started](getting-started.mdx) or have a look in use cases section

