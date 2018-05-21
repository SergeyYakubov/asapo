//+build !test

package server

import (
	log "asapo_discovery/logger"
	"asapo_discovery/utils"
	"net/http"
	"strconv"
)

func Start() {
	mux := utils.NewRouter(listRoutes)
	log.Info("Listening on port: " + strconv.Itoa(settings.Port))
	log.Fatal(http.ListenAndServe(":"+strconv.Itoa(settings.Port), http.HandlerFunc(mux.ServeHTTP)))
}

func ReadConfig(fname string) (log.Level, error) {
	if err := utils.ReadJsonFromFile(fname, &settings); err != nil {
		return log.FatalLevel, err
	}

	if err := settings.Validate(); err != nil {
		return log.FatalLevel,err
	}

	return log.LevelFromString(settings.LogLevel)

}
