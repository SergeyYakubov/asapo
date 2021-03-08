module asapo_discovery

go 1.16

replace asapo_common v0.0.0 => ../../../common/go/src/asapo_common

require (
	asapo_common v0.0.0
	github.com/gorilla/mux v1.7.4 // indirect
	github.com/hashicorp/consul/api v1.4.0
	github.com/sirupsen/logrus v1.5.0 // indirect
	github.com/stretchr/testify v1.4.0
	k8s.io/api v0.17.0
	k8s.io/apimachinery v0.17.0
	k8s.io/client-go v0.17.0
)
