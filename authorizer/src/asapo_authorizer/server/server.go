package server

import (
	"asapo_authorizer/ldap_client"
	"asapo_common/utils"
)

type  beamtimeMeta struct {
	BeamtimeId string  `json:"beamtimeId"`
	Beamline string     `json:"beamline"`
	Stream string       `json:"stream"`
	OfflinePath string `json:"core-path"`
	OnlinePath string `json:"beamline-path"`
	Type string `json:"source-type"`
}

type serverSettings struct {
	Port                    int
	LogLevel                string
	RootBeamtimesFolder     string
	CurrentBeamlinesFolder string
	AlwaysAllowedBeamtimes  []beamtimeMeta
	SecretFile              string
	TokenDurationMin    	int
	Ldap struct {
		Uri string
		BaseDn string
		FilterTemplate string
	}
}

var settings serverSettings
var ldapClient ldap_client.LdapClient
var authHMAC utils.Auth
var authJWT utils.Auth

