package server

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/database"
	"asapo_authorizer/ldap_client"
	"asapo_common/discovery"
	log "asapo_common/logger"
	"errors"
)

type  beamtimeMeta struct {
	BeamtimeId string  `json:"beamtimeId"`
	Beamline string     `json:"beamline"`
	DataSource string       `json:"dataSource"`
	OfflinePath string `json:"corePath"`
	OnlinePath string `json:"beamline-path"`
	Type string `json:"source-type"`
	AccessTypes []string `json:"access-types"`
}

type  commissioningMeta struct {
	Id string  `json:"id"`
	Beamline string     `json:"beamline"`
	OfflinePath string `json:"corePath"`
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
	DiscoveryServer string
	DatabaseServer string
	discoveredDbAddress string
}

func (s *serverSettings) GetDatabaseServer() string {
	if s.DatabaseServer == "auto" {
		return s.discoveredDbAddress
	} else {
		return s.DatabaseServer
	}
}

var settings serverSettings
var ldapClient ldap_client.LdapClient
var Auth *authorization.Auth
var discoveryService discovery.DiscoveryAPI
var db database.Agent

func ReconnectDb() (err error) {
	if db == nil {
		return errors.New("database not initialized")
	}
	db.Close()
	return InitDB(db)
}

func InitDB(dbAgent database.Agent) (err error) {
	db = dbAgent
	if settings.DatabaseServer == "auto" {
		settings.discoveredDbAddress, err = discoveryService.GetMongoDbAddress()
		if err != nil {
			return err
		}
		if settings.discoveredDbAddress == "" {
			return errors.New("no database servers found")
		}
		log.Debug("Got mongodb server: " + settings.discoveredDbAddress)
	}

	return db.Connect(settings.GetDatabaseServer())
}

func CleanupDB() {
	if db != nil {
		db.Close()
	}
}
