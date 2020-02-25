package server

import "asapo_common/utils"

type  beamtimeMeta struct {
	BeamtimeId string  `json:"beamtimeId"`
	Beamline string     `json:"beamline"`
	Stream string       `json:"stream"`
	OfflinePath string `json:"core-path"`
	OnlinePath string `json:"beamline-path"`
}

type serverSettings struct {
	Port                    int
	LogLevel                string
	IpBeamlineMappingFolder string
	RootBeamtimesFolder     string
	CurrentBeamlinesFolder string
	AlwaysAllowedBeamtimes  []beamtimeMeta
	SecretFile              string
}

var settings serverSettings
var auth utils.Auth

