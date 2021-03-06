module asapo_discovery

go 1.16

replace asapo_common v0.0.0 => ../../../common/go/src/asapo_common

require (
	asapo_common v0.0.0
	github.com/hashicorp/consul/api v1.4.0
	github.com/prometheus/client_golang v1.11.0
	github.com/stretchr/testify v1.7.0
	k8s.io/apimachinery v0.17.0
	k8s.io/client-go v0.17.0
)
