module asapo_tools

go 1.16

replace asapo_common v0.0.0 => ../../../common/go/src/asapo_common

require (
	asapo_common v0.0.0
	github.com/kr/pretty v0.2.0 // indirect
	github.com/stretchr/testify v1.7.0
	gopkg.in/check.v1 v1.0.0-20190902080502-41f04d3bba15 // indirect
)
