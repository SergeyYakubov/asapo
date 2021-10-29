# ASAP::O - High performance distributed streaming platform

## Folder structure
 
### Microservices 
 - **authorizer** - authorization _(Go)_
 - **broker** - server for consumer clients _(Go)_
 - **discovery** - discovery service _(Go)_
 - **file_transfer** - file transfer service to deliver files to clients that have no access to storage _(Go)_
 - **receiver** - server for producer clients to send data to, also contains data server that is contacted by consumer clients to get data from cache _(C++)_ 
   
### Client libraries
  - **producer** - producer libraries _(C++, C, Python)_ 
  - **consumer** - consumer libraries _(C++, C, Python)_ 

### Docs & Tests
- **docs** - doxygen,sphinx and site documentation
- **tests** - automatic integration/e2e tests
- **docs/site/examples** - examples used for site     
- **examples** - some outdated examples, also used for automated tests, (todo: move it somewhere)    

### Auxiliary folders
- **common** -  stuff to be shared between microservices
- **CMakeIncludes**, **CMakeModules**, **install** - CMake stuff  
- **config** - to store various config files, also nomad jobs for tests
- **deploy** - Docker files for various images, Helm files, Nomad&Consul files

## Building

### Prepare dependencies

depending on configuration, you might need need curllib, mongoc, google tests

  - cd 3d_party/mongo-c-driver
  - ./install.sh $(pwd)
  - sudo make -C mongo-c-driver-1.17.2 install

  - back in the asapo-dir:
  - mkdir build
  - (cd build; cmake ..)

### With documentation

Need Doxygen >= [1.8.10](https://github.com/doxygen/doxygen/releases/tag/Release_1_8_11)
and sphinx

 - mkdir build
 - cd build
 - cmake -DBUILD_CPP_DOCS=ON _DBUILD_PYTHON_DOCS ..

### With tests

Need googletest >= [1.8.0](https://github.com/google/googletest/releases/tag/release-1.8.0)

 - mkdir build
 - cd build
 - cmake -DBUILD_TESTS=ON ..

The software is MIT licensed (see LICENSE.txt) and uses third party libraries that are distributed under their own terms
(see LICENSE-3RD-PARTY.txt)

### with non-standard 3rd party libraries paths
- cmake -Dlibmongoc-static-1.0_DIR=... -Dlibbson-static-1.0_DIR=...  -Dgtest_SOURCE_DIR=... -DLIBCURL_DIR=... ...

### compile
 - make -j 4
 
or compile specific target only, e.g. 
 - make -j 4 receiver  