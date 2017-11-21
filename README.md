# HIDRA2

# C++ Projects
 
 - /common/cpp
 
 **Library:** Common library which get shared between the producer and receiver 
 
 - /producer/producer-api
 
 **Library:** Producer library which can send data to the receiver
 
 - /receiver
 
 **Executable:** The receiver which can receive data from the producer
  
 - /producer/inotify-event-detector-cpp
 
 **Executable:** Implementation of the producer api with inotify


# Building

## With documentation

Need Doxygen >= [1.8.10](https://github.com/doxygen/doxygen/releases/tag/Release_1_8_11)

 - mkdir build
 - cd build
 - cmake -DBUILD_DOCS=ON ..

## With tests

Need googletest >= [1.8.0](https://github.com/google/googletest/releases/tag/release-1.8.0)

 - mkdir build
 - cd build
 - cmake -DBUILD_TESTS=ON ..
