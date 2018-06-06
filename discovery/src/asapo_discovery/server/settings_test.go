package server

import (
	"github.com/stretchr/testify/assert"
	"testing"
	"asapo_discovery/utils"
)

func fillSettings(mode string) utils.Settings {
	var settings utils.Settings
	settings.Port = 1
	settings.Mode = mode
	settings.Receiver.MaxConnections = 10
	settings.LogLevel = "info"
	settings.Receiver.ForceEndpoints=[]string{"ip1","ip2"}
	settings.Broker.ForceEndpoint="ip_b"
	settings.ConsulEndpoints=[]string{"ipc1","ipc2"}
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
	settings.Receiver.ForceEndpoints=[]string{}
	err := settings.Validate()
	assert.NotNil(t, err)
}

func TestSettingsStaticModeNoBrokerEndpoints(t *testing.T) {
	settings := fillSettings("static")
	settings.Broker.ForceEndpoint=""
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
