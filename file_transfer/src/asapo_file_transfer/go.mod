module asapo_file_transfer

go 1.16

replace asapo_common v0.0.0 => ../../../common/go/src/asapo_common

require (
	asapo_common v0.0.0
	github.com/stretchr/testify v1.7.0
	google.golang.org/grpc v1.39.0 // indirect
)
