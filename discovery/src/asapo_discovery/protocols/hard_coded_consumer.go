package protocols

import "time"

func getTimefromDate(date string) time.Time{
	res,err := time.Parse("2006-01-02", date)
	if err!=nil {
		panic(err)
	}
	return res
}

func GetSupportedConsumerProtocols() []Protocol {
	return []Protocol{
		Protocol{"v0.5",
			map[string]string{
				"Discovery": "v0.1",
				"Authorizer": "v0.2",
				"Broker": "v0.5",
				"File Transfer": "v0.2",
				"Data cache service": "v0.1",
			}, &protocolValidatorCurrent{}},
		Protocol{"v0.4",
			map[string]string{
				"Discovery": "v0.1",
				"Authorizer": "v0.2",
				"Broker": "v0.4",
				"File Transfer": "v0.2",
				"Data cache service": "v0.1",
			}, &protocolValidatorDeprecated{getTimefromDate("2022-12-01")}},
		Protocol{"v0.3",
			map[string]string{
				"Discovery": "v0.1",
				"Authorizer": "v0.1",
				"Broker": "v0.3",
				"File Transfer": "v0.1",
				"Data cache service": "v0.1",
			}, &protocolValidatorDeprecated{getTimefromDate("2022-07-01")}},
		Protocol{"v0.2",
			map[string]string{
				"Discovery": "v0.1",
				"Authorizer": "v0.1",
				"Broker": "v0.2",
				"File Transfer": "v0.1",
				"Data cache service": "v0.1",
			}, &protocolValidatorDeprecated{getTimefromDate("2022-06-01")}},
		Protocol{"v0.1",
			map[string]string{
				"Discovery": "v0.1",
				"Authorizer": "v0.1",
				"Broker": "v0.1",
				"File Transfer": "v0.1",
				"Data cache service": "v0.1",
			}, &protocolValidatorDeprecated{getTimefromDate("2022-06-01")}},
	}
}
