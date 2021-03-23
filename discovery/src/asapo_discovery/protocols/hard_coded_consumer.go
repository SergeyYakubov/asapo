package protocols

func GetSupportedConsumerProtocols() []Protocol {
	return []Protocol{
		Protocol{"v0.1",&protocolValidatorCurrent{}},
	}
}



