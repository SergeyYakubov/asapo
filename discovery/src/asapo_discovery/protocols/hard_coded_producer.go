package protocols

func GetSupportedProducerProtocols() []Protocol {
	return []Protocol{
		Protocol{"v0.1",&protocolValidatorCurrent{}},
	}
}



