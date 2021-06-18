module asapo_authorizer

go 1.16

replace asapo_common v0.0.0 => ../../../common/go/src/asapo_common

require (
	asapo_common v0.0.0
	github.com/go-ldap/ldap v3.0.3+incompatible
	github.com/rs/xid v1.2.1
	github.com/stretchr/testify v1.7.0
	gopkg.in/asn1-ber.v1 v1.0.0-20181015200546-f715ec2f112d // indirect
)
