package server


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
}

var settings serverSettings

