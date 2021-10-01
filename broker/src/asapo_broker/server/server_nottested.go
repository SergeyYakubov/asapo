//+build !test

package server

import (
	log "asapo_common/logger"
	"asapo_common/utils"
	"errors"
	"net/http"
	"strconv"
	_ "net/http/pprof"
)

func StartStatistics() {
	statistics.Writer = new(StatisticInfluxDbWriter)
	statistics.Init()
	statistics.Reset()
	go statistics.Monitor()
}

func Start() {
	if settings.MonitorPerformance {
		StartStatistics()
	}
	mux := utils.NewRouter(listRoutes)
	mux.PathPrefix("/debug/pprof/").Handler(http.DefaultServeMux)
	log.Info("Listening on port: " + strconv.Itoa(settings.Port))
	log.Fatal(http.ListenAndServe(":"+strconv.Itoa(settings.Port), http.HandlerFunc(mux.ServeHTTP)))
}

func createAuth() Authorizer {
	return &AsapoAuthorizer{settings.AuthorizationServer,&http.Client{}}
}

func ReadConfig(fname string) (log.Level, error) {
	if err := utils.ReadJsonFromFile(fname, &settings); err != nil {
		return log.FatalLevel, err
	}

	if settings.DatabaseServer == "" {
		return log.FatalLevel, errors.New("DatabaseServer not set")
	}

	if settings.PerformanceDbServer == "" {
		return log.FatalLevel, errors.New("PerformanceDbServer not set")
	}

	if settings.DatabaseServer == "auto" && settings.DiscoveryServer == "" {
		return log.FatalLevel, errors.New("DiscoveryServer not set for auto DatabaseServer")
	}

	if settings.Port == 0 {
		return log.FatalLevel, errors.New("Server port not set")
	}

	if settings.CheckResendInterval==nil || *settings.CheckResendInterval<0  {
		return log.FatalLevel, errors.New("Resend interval must be set and not negative")
	}

	if settings.PerformanceDbName == "" {
		return log.FatalLevel, errors.New("PerformanceDbName not set")
	}

	if settings.AuthorizationServer == "" {
		return log.FatalLevel, errors.New("AuthorizationServer not set")
	}

	auth = createAuth()

	return log.LevelFromString(settings.LogLevel)
}
