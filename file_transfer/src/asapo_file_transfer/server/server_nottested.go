//+build !test

package server

import (
	log "asapo_common/logger"
	"asapo_common/utils"
	"asapo_common/version"
	"errors"
	"net/http"
	"strconv"
)

func Start() {
	mux := utils.NewRouter(listRoutes)
	log.Info("Starting ASAPO Authorizer, version " + version.GetVersion())
	log.Info("Listening on port: " + strconv.Itoa(settings.Port))
	log.Fatal(http.ListenAndServe(":"+strconv.Itoa(settings.Port), utils.ProcessJWTAuth(mux.ServeHTTP, settings.key)))
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
	settings.key, err = utils.ReadFirstStringFromFile(settings.SecretFile)
	if err != nil {
		return log.FatalLevel, errors.New("Cannot read secret from file " + err.Error())
	}

	level, err := log.LevelFromString(settings.LogLevel)

	return level, err
}
