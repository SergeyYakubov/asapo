package common

type BeamtimeMeta struct {
	InstanceId string  `json:"instanceId"`
	PipelineStep string     `json:"pipelineStep"`
	BeamtimeId string  `json:"beamtimeId"`
	Beamline string     `json:"beamline"`
	DataSource string       `json:"dataSource"`
	OfflinePath string `json:"corePath"`
	OnlinePath string `json:"beamline-path"`
	Type string `json:"source-type"`
	AccessTypes []string `json:"access-types"`
}

type CommissioningMeta struct {
	Id string  `json:"id"`
	Beamline string     `json:"beamline"`
	OfflinePath string `json:"corePath"`
}
