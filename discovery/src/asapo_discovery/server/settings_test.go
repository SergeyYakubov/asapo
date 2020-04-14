package server

import (
	"github.com/stretchr/testify/assert"
	"testing"
	"asapo_discovery/common"
)

func fillSettings(mode string) common.Settings {
	var settings common.Settings
	settings.Port = 1
	settings.Mode = mode
	settings.Receiver.MaxConnections = 10
	settings.LogLevel = "info"
	settings.Receiver.StaticEndpoints=[]string{"ip1","ip2"}
	settings.Broker.StaticEndpoint="ip_b"
	settings.Mongo.StaticEndpoint="ip_m"
	settings.ConsulEndpoints=[]string{"ipc1","ipc2"}
	settings.Kubernetes.ConfigFile=""
	settings.Kubernetes.Mode="external"
	return settings
}

func TestSettingsOK(t *testing.T) {
	settings := fillSettings("static")
	err := settings.Validate()
	assert.Nil(t, err)
}

func TestSettingsWrongMode(t *testing.T) {
	settings := fillSettings("blalba")
	err := settings.Validate()
	assert.NotNil(t, err)
}

func TestSettingsStaticModeNoReceiverEndpoints(t *testing.T) {
	settings := fillSettings("static")
	settings.Receiver.StaticEndpoints=[]string{}
	err := settings.Validate()
	assert.NotNil(t, err)
}

func TestSettingsStaticModeNoBrokerEndpoints(t *testing.T) {
	settings := fillSettings("static")
	settings.Broker.StaticEndpoint=""
	err := settings.Validate()
	assert.NotNil(t, err)
}

func TestSettingsStaticModeNoMongoEndpoints(t *testing.T) {
	settings := fillSettings("static")
	settings.Mongo.StaticEndpoint=""
	err := settings.Validate()
	assert.NotNil(t, err)
}


func TestSettingsConsulModeNoEndpoints(t *testing.T) {
	settings := fillSettings("consul")
	settings.ConsulEndpoints=[]string{}
	err := settings.Validate()
	assert.Nil(t, err)
}

func TestGetHandlerMode(t *testing.T) {
	mode := "consul"
	settings = fillSettings(mode)
	assert.Equal(t,mode,GetHandlerMode())
}

func TestSettingsOKKubernetes(t *testing.T) {
	settings := fillSettings("kubernetes")
	err := settings.Validate()
	assert.Nil(t, err)
}
