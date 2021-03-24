package protocols

func GetSupportedConsumerProtocols() []Protocol {
	return []Protocol{
		Protocol{"v0.1",
			map[string]string{
				"Discovery": "v0.1",
				"Authorizer": "v0.1",
				"Broker": "v0.1",
				"File Transfer": "v0.1",
				"Data cache service": "v0.1",
			}, &protocolValidatorCurrent{}},
	}
}
