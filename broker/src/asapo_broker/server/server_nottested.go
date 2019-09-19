//+build !test

package server

import (
	log "asapo_common/logger"
	"asapo_common/utils"
	"errors"
	"net/http"
	"strconv"
)

func StartStatistics() {
	statistics.Writer = new(StatisticInfluxDbWriter)
	statistics.Init()
	statistics.Reset()
	go statistics.Monitor()
}

func Start() {
	StartStatistics()
	mux := utils.NewRouter(listRoutes)
	log.Info("Listening on port: " + strconv.Itoa(settings.Port))
	log.Fatal(http.ListenAndServe(":"+strconv.Itoa(settings.Port), http.HandlerFunc(mux.ServeHTTP)))
}

func createAuth() (utils.Auth, error) {
	secret, err := utils.ReadFirstStringFromFile(settings.SecretFile)
	if err != nil {
		return nil, err
	}
	return utils.NewHMACAuth(secret), nil
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

	if settings.PerformanceDbName == "" {
		return log.FatalLevel, errors.New("PerformanceDbName not set")
	}

	if settings.SecretFile == "" {
		return log.FatalLevel, errors.New("Secret file not set")
	}

	var err error
	auth, err = createAuth()
	if err != nil {
		return log.FatalLevel, err
	}

	level, err := log.LevelFromString(settings.LogLevel)

	return level, err
}
