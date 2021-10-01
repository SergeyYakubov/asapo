//+build !test

package server

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/common"
	"asapo_authorizer/ldap_client"
	"asapo_authorizer/token_store"
	log "asapo_common/logger"
	"asapo_common/utils"
	"asapo_common/version"
	"errors"
	"net/http"
	_ "net/http/pprof"
	"strconv"
)

func Start() {
	mux := utils.NewRouter(listRoutes)
	ldapClient = new(ldap_client.OpenLdapClient)

	log.Info("Starting ASAPO Authorizer, version " + version.GetVersion())

	store = new(token_store.TokenStore)
	err := 	store.Init(nil)
	if err != nil {
		log.Error(err.Error())
	}
	defer store.Close()

	log.Info("Listening on port: " + strconv.Itoa(common.Settings.Port))
	mux.PathPrefix("/debug/pprof/").Handler(http.DefaultServeMux)
	log.Fatal(http.ListenAndServe(":"+strconv.Itoa(common.Settings.Port), http.HandlerFunc(mux.ServeHTTP)))
}

func createAuth() (*authorization.Auth,error) {
	secret, err := utils.ReadFirstStringFromFile(common.Settings.UserSecretFile)
	if err != nil {
		return nil, err
	}
	adminSecret, err := utils.ReadFirstStringFromFile(common.Settings.AdminSecretFile)
	if err != nil {
		return nil, err
	}
	return authorization.NewAuth(utils.NewJWTAuth(secret), utils.NewJWTAuth(adminSecret), utils.NewJWTAuth(secret)),nil
}

func ReadConfig(fname string) (log.Level, error) {
	if err := utils.ReadJsonFromFile(fname, &common.Settings); err != nil {
		return log.FatalLevel, err
	}

	if common.Settings.Port == 0 {
		return log.FatalLevel, errors.New("Server port not set")
	}

	if common.Settings.UserSecretFile == "" {
		return log.FatalLevel, errors.New("User secret file not set")
	}

	if common.Settings.AdminSecretFile == "" {
		return log.FatalLevel, errors.New("Admin secret file not set")
	}

	if common.Settings.DatabaseServer == "" {
		return log.FatalLevel, errors.New("Database server not set")
	}

	if common.Settings.DatabaseServer == "auto" && common.Settings.DiscoveryServer=="" {
		return log.FatalLevel, errors.New("Discovery server not set")
	}

	var err error
	Auth, err = createAuth()
	if err != nil {
		return log.FatalLevel, err
	}

	level, err := log.LevelFromString(common.Settings.LogLevel)

	return level, err
}
