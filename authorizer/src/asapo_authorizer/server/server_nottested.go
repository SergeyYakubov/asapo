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
	log.Fatal(http.ListenAndServe(":"+strconv.Itoa(settings.Port), http.HandlerFunc(mux.ServeHTTP)))
}

func ReadConfig(fname string) (log.Level, error) {
	if err := utils.ReadJsonFromFile(fname, &settings); err != nil {
		return log.FatalLevel, err
	}

	if settings.Port == 0 {
		return log.FatalLevel, errors.New("Server port not set")
	}

	level, err := log.LevelFromString(settings.LogLevel)

	return level, err
}
