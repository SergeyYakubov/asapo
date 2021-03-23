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
	validator protocolValidator
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

func ValidateProtocol(client string, version string) (hint string, ok bool) {
	protocols, err := getSupportedProtocols(client)
	if err != nil {
		return err.Error(), false
	}
	for _, protocol := range protocols {
		if protocol.Version == version {
			return protocol.IsValid()
		}
	}
	return "unknown protocol", false
}

func GetSupportedProtocolsArray(client string) ([]string, error) {
	protocols,err := getSupportedProtocols(client)
	if err!=nil  {
		return nil,err
	}
	res:=make([]string,0)
	for _,protocol := range protocols {
		res = append(res, protocol.GetString())
	}
	return res,nil
}