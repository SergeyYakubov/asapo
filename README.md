# asapo

# C++ Projects
 
 - /common/cpp
 
 **Library:** Common library which get shared between the producer and receiver 
 
 - /producer/asapo-producer
 
 **Library:** Producer library which can send data to the receiver
 
 - /receiver
 
 **Executable:** The receiver which can receive data from the producer
  
 - /producer/inotify-event-detector-cpp
 
 **Executable:** Implementation of the producer api with inotify


# Building

## Prepare dependencies

depending on configuration, you might need need curllib, mongoc, google tests

  - cd 3d_party/mongo-c-driver
  - ./install.sh $(pwd)
  - sudo make -C mongo-c-driver-1.17.2 install

  - back in the asapo-dir:
  - mkdir build
  - (cd build; cmake ..)

## With documentation

Need Doxygen >= [1.8.10](https://github.com/doxygen/doxygen/releases/tag/Release_1_8_11)
and sphinx

 - mkdir build
 - cd build
 - cmake -DBUILD_CPP_DOCS=ON _DBUILD_PYTHON_DOCS ..

## With tests

Need googletest >= [1.8.0](https://github.com/google/googletest/releases/tag/release-1.8.0)

 - mkdir build
 - cd build
 - cmake -DBUILD_TESTS=ON ..

The software is MIT licensed (see LICENSE.txt) and uses third party libraries that are distributed under their own terms
(see LICENSE-3RD-PARTY.txt)

## with non-standard 3rd party libraries paths
- cmake -Dlibmongoc-static-1.0_DIR=... -Dlibbson-static-1.0_DIR=...  -Dgtest_SOURCE_DIR=... -DLIBCURL_DIR=... ...

## compile
 - make -j 4
 
or compile specific target only, e.g. 
 - make -j 4 receiver  