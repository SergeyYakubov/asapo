module asapo_broker

go 1.16

replace asapo_common v0.0.0 => ../../../common/go/src/asapo_common

require (
	asapo_common v0.0.0
	github.com/blastrain/vitess-sqlparser v0.0.0-20201030050434-a139afbb1aba
	github.com/gorilla/mux v1.8.0
	github.com/influxdata/influxdb1-client v0.0.0-20200827194710-b269163b24ab
	github.com/rs/xid v1.2.1
	github.com/stretchr/testify v1.7.0
	go.mongodb.org/mongo-driver v1.5.3
	google.golang.org/grpc v1.39.0 // indirect
)
