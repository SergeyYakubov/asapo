#!/bin/bash

set -e

PROTO_INPUT_FOLDER=../../../networking/monitoring/
PROTO_INPUT=$PROTO_INPUT_FOLDER*.proto
PROTO_DEST=./generated_proto

rm -rf ${PROTO_DEST}
mkdir -p ${PROTO_DEST}

GO111MODULE=off go get \
    google.golang.org/protobuf/cmd/protoc-gen-go \
    google.golang.org/grpc/cmd/protoc-gen-go-grpc

PATH="$PATH:$(go env GOPATH)/bin" protoc \
    --go_out=generated_proto --go_opt=paths=source_relative \
    --go-grpc_out=generated_proto --go-grpc_opt=paths=source_relative \
    -I ${PROTO_INPUT_FOLDER} \
    ${PROTO_INPUT}

go get google.golang.org/grpc
