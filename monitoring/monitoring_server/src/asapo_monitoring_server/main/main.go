package main

import (
	log "asapo_common/logger"
	"asapo_common/utils"
	"asapo_common/version"
	"asapo_monitoring_server/server"
	"errors"
	"flag"
	"os"
)

func PrintUsage() {
	log.Fatal("Usage: " + os.Args[0] + " -config <config file>")
}

func loadConfig(configFileName string) (log.Level, server.Settings, error) {
	var settings server.Settings
	if err := utils.ReadJsonFromFile(configFileName, &settings); err != nil {
		return log.FatalLevel, server.Settings{}, err
	}

	if settings.ThisClusterName == "" {
		return log.FatalLevel, server.Settings{}, errors.New("'ThisClusterName' not set")
	}

	if settings.ServerPort == 0 {
		return log.FatalLevel, server.Settings{}, errors.New("'ServerPort' not set")
	}

	if settings.LogLevel == "" {
		return log.FatalLevel, server.Settings{}, errors.New("'LogLevel' not set")
	}

	if settings.InfluxDb2Url == "" {
		return log.FatalLevel, server.Settings{}, errors.New("'InfluxDb2Url' not set")
	}

	if settings.InfluxDb2AuthToken == "" {
		return log.FatalLevel, server.Settings{}, errors.New("'InfluxDb2AuthToken' not set")
	}

	if settings.InfluxDb2Org == "" {
		return log.FatalLevel, server.Settings{}, errors.New("'InfluxDb2Org' not set")
	}

	if settings.InfluxDb2Bucket == "" {
		return log.FatalLevel, server.Settings{}, errors.New("'InfluxDb2Bucket' not set")
	}

	logLevel, err := log.LevelFromString(settings.LogLevel)
	if err != nil {
		return log.FatalLevel, server.Settings{}, err
	}

	return logLevel, settings, nil
}

func main() {
	var configFileName = flag.String("config", "", "config file path")

	if ret := version.ShowVersion(os.Stdout, "ASAPO Monitoring Server"); ret {
		return
	}

	path, err := os.Getwd()
	println("oath: " + path)

	log.SetSoucre("monitoring server")

	flag.Parse()
	if *configFileName == "" {
		PrintUsage()
	}

	logLevel, settings, err := loadConfig(*configFileName)
	if err != nil {
		log.Fatal(err.Error())
		return
	}

	log.SetLevel(logLevel)

	server.Start(settings)
}
