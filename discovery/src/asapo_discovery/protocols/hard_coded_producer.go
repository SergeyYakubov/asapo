package protocols

func GetSupportedProducerProtocols() []Protocol {
	return []Protocol{
		Protocol{"v0.4",
			map[string]string{
				"Discovery": "v0.1",
				"Receiver": "v0.4",
			}, &protocolValidatorCurrent{}},
		Protocol{"v0.3",
			map[string]string{
				"Discovery": "v0.1",
				"Receiver": "v0.3",
			}, &protocolValidatorCurrent{}},
		Protocol{"v0.2",
			map[string]string{
				"Discovery": "v0.1",
				"Receiver": "v0.2",
			}, &protocolValidatorDeprecated{getTimefromDate("2022-07-01")}},
		Protocol{"v0.1",
			map[string]string{
				"Discovery": "v0.1",
				"Receiver": "v0.1",
			}, &protocolValidatorDeprecated{getTimefromDate("2022-06-01")}},
	}
}



