//+build !test

package server

import (
	log "asapo_common/logger"
	"asapo_common/utils"
	"asapo_common/version"
	"github.com/prometheus/client_golang/prometheus"
	"github.com/prometheus/client_golang/prometheus/promhttp"
	"net/http"
	_ "net/http/pprof"
	"strconv"
)

func init() {
	prometheus.MustRegister(nReqests)
}

func Start() {
	mux := utils.NewRouter(listRoutes)
	mux.PathPrefix("/debug/pprof/").Handler(http.DefaultServeMux)
	mux.PathPrefix("/metrics").Handler(promhttp.Handler())

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
