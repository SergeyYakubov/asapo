//+build !test

package server

import (
	log "asapo_common/logger"
	"asapo_common/utils"
    "asapo_common/version"
	"net/http"
	"strconv"
	_ "net/http/pprof"
)

func Start() {
	mux := utils.NewRouter(listRoutes)
	mux.PathPrefix("/debug/pprof/").Handler(http.DefaultServeMux)
	log.Info("Starting ASAPO Discovery, version " + version.GetVersion())
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
