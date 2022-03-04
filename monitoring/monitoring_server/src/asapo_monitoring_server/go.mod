module asapo_monitoring_server

go 1.16

replace asapo_common v0.0.0 => ../../../../common/go/src/asapo_common

require (
	asapo_common v0.0.0
	github.com/golang/protobuf v1.5.2 // indirect
	github.com/influxdata/influxdb-client-go/v2 v2.4.0
	google.golang.org/grpc v1.39.0
	google.golang.org/grpc/cmd/protoc-gen-go-grpc v1.1.0 // indirect
	google.golang.org/protobuf v1.27.1
)
