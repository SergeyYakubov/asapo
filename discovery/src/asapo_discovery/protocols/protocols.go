package protocols

import "errors"

type protocolValidator interface {
	IsValid() (hint string, ok bool)
}

type protocolValidatorCurrent struct {
}

func (p *protocolValidatorCurrent) IsValid() (hint string, ok bool) {
	return "current", true
}

type Protocol struct {
	Version   string
	MicroserviceAPis map[string]string
	validator protocolValidator
}

type ProtocolInfo  struct {
	Info string
	MicroserviceAPis map[string]string
}

func (p *Protocol) IsValid() (hint string, ok bool) {
	return p.validator.IsValid()
}

func (p *Protocol) GetString() string {
	hint, _ := p.validator.IsValid()
	if hint != "" {
		return p.Version + " (" + hint + ")"
	} else {
		return p.Version
	}
}

func getSupportedProtocols(client string) ([]Protocol, error) {
	switch client {
	case "consumer":
		return GetSupportedConsumerProtocols(), nil
	case "producer":
		return GetSupportedProducerProtocols(), nil
	}
	return nil, errors.New("unknown client")
}

func FindProtocol(client string, version string) (Protocol, error) {
	protocols, err := getSupportedProtocols(client)
	if err != nil {
		return Protocol{},err
	}
	for _, protocol := range protocols {
		if protocol.Version == version {
			return protocol,nil
		}
	}
	return Protocol{},errors.New("unknown protocol")
}

func ValidateProtocol(client string, version string) (hint string, ok bool) {
	protocol, err := FindProtocol(client,version)
	if err != nil {
		return err.Error(), false
	}
	return protocol.IsValid()
}

func GetSupportedProtocolsArray(client string) ([]ProtocolInfo, error) {
	protocols,err := getSupportedProtocols(client)
	if err!=nil  {
		return nil,err
	}
	res:=make([]ProtocolInfo,0)
	for _,protocol := range protocols {
		var info ProtocolInfo
		info.Info = protocol.GetString()
		info.MicroserviceAPis = protocol.MicroserviceAPis
		res = append(res, info)
	}
	return res,nil
}