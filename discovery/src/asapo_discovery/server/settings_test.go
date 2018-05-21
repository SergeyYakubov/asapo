package server

import (
	"github.com/stretchr/testify/assert"
	"testing"
)

func fillSettings(mode string)serverSettings {
	var settings serverSettings
	settings.Port = 1
	settings.Mode = mode
	settings.MaxConnections = 10
	settings.LogLevel = "info"
	settings.Endpoints=[]string{"ip1","ip2"}
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

func TestSettingsStaticModeNoEndpoints(t *testing.T) {
	settings := fillSettings("static")
	settings.Endpoints=[]string{}
	err := settings.Validate()
	assert.NotNil(t, err)
}

func TestSettingsConsulModeNoEndpoints(t *testing.T) {
	settings := fillSettings("consul")
	settings.Endpoints=[]string{}
	err := settings.Validate()
	assert.Nil(t, err)
}

func TestGetHandlerMode(t *testing.T) {
	mode := "consul"
	settings = fillSettings(mode)
	assert.Equal(t,mode,GetHandlerMode())
}
