package server

import "asapo_common/utils"

type  beamtimeInfo struct {
	BeamtimeId string
	Beamline string
	Stream string
	Year string
	Facility string
}

type serverSettings struct {
	Port                    int
	LogLevel                string
	IpBeamlineMappingFolder string
	RootBeamtimesFolder     string
	CurrentBeamlinesFolder string
	AlwaysAllowedBeamtimes  []beamtimeInfo
	SecretFile              string
}

var settings serverSettings
var auth utils.Auth

