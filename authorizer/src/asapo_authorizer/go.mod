module asapo_authorizer

go 1.16

replace asapo_common v0.0.0 => ../../../common/go/src/asapo_common

require (
	asapo_common v0.0.0
	github.com/dgrijalva/jwt-go v3.2.0+incompatible // indirect
	github.com/go-ldap/ldap v3.0.3+incompatible
	github.com/gorilla/mux v1.8.0 // indirect
	github.com/sirupsen/logrus v1.8.0 // indirect
	github.com/stretchr/testify v1.7.0
	gopkg.in/asn1-ber.v1 v1.0.0-20181015200546-f715ec2f112d // indirect
)
