package server

import "asapo_common/utils"

type  beamtimeInfo struct {
	BeamtimeId string
	Beamline string
	Stream string
}

type serverSettings struct {
	Port             int
	LogLevel         string
	IpBeamlineMappingFolder string
	BeamtimeBeamlineMappingFile string
	AlwaysAllowedBeamtimes []beamtimeInfo
	SecretFile       string
}

var settings serverSettings
var auth utils.Auth

