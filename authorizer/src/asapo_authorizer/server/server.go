package server

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/common"
	"asapo_authorizer/ldap_client"
	"asapo_authorizer/token_store"
	"sync"
)

var ldapClient ldap_client.LdapClient
var Auth *authorization.Auth
var store token_store.Store
var cachedMetas = struct {
	cache map[string]common.BeamtimeMeta
	lock  sync.Mutex
}{cache: make(map[string]common.BeamtimeMeta, 0)}
