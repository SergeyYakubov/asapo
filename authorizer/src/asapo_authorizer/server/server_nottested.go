//+build !test

package server

import (
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
	ldapClient = new (ldap_client.OpenLdapClient)
	log.Info("Starting ASAPO Authorizer, version " + version.GetVersion())
	log.Info("Listening on port: " + strconv.Itoa(settings.Port))
	log.Fatal(http.ListenAndServe(":"+strconv.Itoa(settings.Port), http.HandlerFunc(mux.ServeHTTP)))
}

func createAuth() (utils.Auth,utils.Auth, error) {
	secret, err := utils.ReadFirstStringFromFile(settings.SecretFile)
	if err != nil {
		return nil,nil, err
	}
	return utils.NewHMACAuth(secret),utils.NewJWTAuth(secret), nil
}


func ReadConfig(fname string) (log.Level, error) {
	if err := utils.ReadJsonFromFile(fname, &settings); err != nil {
		return log.FatalLevel, err
	}

	if settings.Port == 0 {
		return log.FatalLevel, errors.New("Server port not set")
	}

	if settings.SecretFile == "" {
		return log.FatalLevel, errors.New("Secret file not set")
	}

	var err error
	authHMAC,authJWT, err = createAuth()
	if err != nil {
		return log.FatalLevel, err
	}

	level, err := log.LevelFromString(settings.LogLevel)

	return level, err
}
