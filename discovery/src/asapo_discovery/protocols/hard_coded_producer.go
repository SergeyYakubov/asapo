package protocols

func GetSupportedProducerProtocols() []Protocol {
	return []Protocol{
		Protocol{"v0.2",
			map[string]string{
				"Discovery": "v0.1",
				"Receiver": "v0.2",
			}, &protocolValidatorCurrent{}},
		Protocol{"v0.1",
			map[string]string{
				"Discovery": "v0.1",
				"Receiver": "v0.1",
			}, &protocolValidatorDeprecated{getTimefromDate("2022-06-01")}},
	}
}



