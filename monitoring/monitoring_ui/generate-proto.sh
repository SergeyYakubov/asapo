#!/bin/bash

PROTO_INPUT_FOLDER=../../common/networking/monitoring/
PROTO_INPUT=$PROTO_INPUT_FOLDER*.proto
PROTO_DEST=./src/generated_proto

rm -rf ${PROTO_DEST}
mkdir -p ${PROTO_DEST}


#npx grpc_tools_node_protoc \
#    --grpc_out="grpc_js:${PROTO_DEST}" \
#    --grpc-web_out="import_style=commonjs+dts,mode=grpcwebtext:${PROTO_DEST}" \
#    #--ts_out="grpc_js:${PROTO_DEST}" \
#    -I ${PROTO_INPUT_FOLDER} \
#    ${PROTO_INPUT}

# Hmm, somehow this client does not work when using binary, and when using grpcwebtext
npx grpc_tools_node_protoc \
    --js_out="import_style=commonjs,binary:${PROTO_DEST}" \
    --grpc-web_out="import_style=commonjs+dts,mode=grpcwebtext:${PROTO_DEST}" \
    -I ${PROTO_INPUT_FOLDER} \
    ${PROTO_INPUT}

# JavaScript code generation
#npx grpc_tools_node_protoc \
#    --grpc-web_out="import_style=typescript,mode=grpcwebtext:${PROTO_DEST}" \
#    --plugin=protoc-gen-grpc=./node_modules/.bin/grpc_tools_node_protoc_plugin \
#    -I ${PROTO_INPUT_FOLDER} \
#    ${PROTO_INPUT}
    #--js_out="import_style=typescript,binary:${PROTO_DEST}" \
    #--ts_out="grpc_js:${PROTO_DEST}" \

# TypeScript code generation
#npx grpc_tools_node_protoc \
#    --plugin=protoc-gen-ts=./node_modules/.bin/protoc-gen-ts \
#    --ts_out="mode=grpc-js:${PROTO_DEST}" \
#    -I ${PROTO_INPUT_FOLDER} \
#    ${PROTO_INPUT}
