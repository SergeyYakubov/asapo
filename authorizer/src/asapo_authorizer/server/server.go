package server

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/ldap_client"
	"asapo_authorizer/token_store"
)

var ldapClient ldap_client.LdapClient
var Auth *authorization.Auth
var store token_store.Store
