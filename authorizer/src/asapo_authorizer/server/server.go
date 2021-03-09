package server

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/ldap_client"
)

type  beamtimeMeta struct {
	BeamtimeId string  `json:"beamtimeId"`
	Beamline string     `json:"beamline"`
	DataSource string       `json:"dataSource"`
	OfflinePath string `json:"core-path"`
	OnlinePath string `json:"beamline-path"`
	Type string `json:"source-type"`
	AccessType string `json:"access-type"`
}

type serverSettings struct {
	Port                   int
	LogLevel               string
	RootBeamtimesFolder    string
	CurrentBeamlinesFolder string
	AlwaysAllowedBeamtimes []beamtimeMeta
	UserSecretFile         string
	AdminSecretFile        string
	FolderTokenDurationMin int
	Ldap                   struct {
		Uri string
		BaseDn string
		FilterTemplate string
	}
}

var settings serverSettings
var ldapClient ldap_client.LdapClient
var Auth *authorization.Auth

