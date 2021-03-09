//+build !test

package server

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/ldap_client"
	log "asapo_common/logger"
	"asapo_common/utils"
	"asapo_common/version"
	"errors"
	"net/http"
	"strconv"
)

func Start() {
	mux := utils.NewRouter(listRoutes)
	ldapClient = new(ldap_client.OpenLdapClient)
	log.Info("Starting ASAPO Authorizer, version " + version.GetVersion())
	log.Info("Listening on port: " + strconv.Itoa(settings.Port))
	log.Fatal(http.ListenAndServe(":"+strconv.Itoa(settings.Port), http.HandlerFunc(mux.ServeHTTP)))
}

func createAuth() (*authorization.Auth,error) {
	secret, err := utils.ReadFirstStringFromFile(settings.UserSecretFile)
	if err != nil {
		return nil, err
	}
	adminSecret, err := utils.ReadFirstStringFromFile(settings.AdminSecretFile)
	if err != nil {
		return nil, err
	}

	return authorization.NewAuth(utils.NewHMACAuth(adminSecret), utils.NewHMACAuth(secret), utils.NewJWTAuth(secret)),nil
}

func ReadConfig(fname string) (log.Level, error) {
	if err := utils.ReadJsonFromFile(fname, &settings); err != nil {
		return log.FatalLevel, err
	}

	if settings.Port == 0 {
		return log.FatalLevel, errors.New("Server port not set")
	}

	if settings.UserSecretFile == "" {
		return log.FatalLevel, errors.New("User secret file not set")
	}

	if settings.AdminSecretFile == "" {
		return log.FatalLevel, errors.New("Admin secret file not set")
	}

	var err error
	Auth, err = createAuth()
	if err != nil {
		return log.FatalLevel, err
	}

	level, err := log.LevelFromString(settings.LogLevel)

	return level, err
}
