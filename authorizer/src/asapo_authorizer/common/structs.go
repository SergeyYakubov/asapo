package common

type BeamtimeMeta struct {
	BeamtimeId string  `json:"beamtimeId"`
	Beamline string     `json:"beamline"`
	DataSource string       `json:"dataSource"`
	OfflinePath string `json:"corePath"`
	OnlinePath string `json:"beamline-path"`
	Type string `json:"source-type"`
	AccessTypes []string `json:"access-types"`
	InstanceId string  `json:"instanceId,omitempty"`
	PipelineStep string     `json:"pipelineStep,omitempty"`
}

type CommissioningMeta struct {
	Id string  `json:"id"`
	Beamline string     `json:"beamline"`
	OfflinePath string `json:"corePath"`
}
