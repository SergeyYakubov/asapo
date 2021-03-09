package cli

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/server"
	"errors"
	"fmt"
	"os"
)

type tokenFlags struct {
	Type       string
	AccessType string
	Beamtime   string
	Beamline   string
	DaysValid  int
}

func userTokenRequest(flags tokenFlags) (request authorization.TokenRequest, err error) {
	if (flags.Beamline=="" && flags.Beamtime=="") || (flags.Beamline!="" && flags.Beamtime!="") {
		return request,errors.New("beamtime or beamline must be set")
	}
	if flags.AccessType!="read" && flags.AccessType!="write" {
		return request,errors.New("access type must be read of write")
	}

	if flags.DaysValid<=0 {
		return request,errors.New("expiration period must be set")
	}

	request.Subject = make(map[string]string,1)
	if (flags.Beamline!="") {
		request.Subject["beamline"]=flags.Beamline
	} else {
		request.Subject["beamtimeId"]=flags.Beamtime
	}
	request.AccessType = flags.AccessType
	request.DaysValid = flags.DaysValid

	return
}


func adminTokenRequest(flags tokenFlags) (request authorization.TokenRequest, err error) {
	if flags.Beamline+flags.Beamtime!="" {
		return request,errors.New("beamtime and beamline must not be set for admin token")
	}
	if flags.AccessType!="create" && flags.AccessType!="revoke" && flags.AccessType!="list" {
		return request,errors.New("access type must be create,revoke of list")
	}
	request.Subject = make(map[string]string,1)
	request.Subject["user"]="admin"
	request.AccessType = flags.AccessType
	request.DaysValid = flags.DaysValid

	return
}

func (cmd *command) CommandCreate_token() (err error) {
	message_string := "Generate token"
	if cmd.description(message_string) {
		return nil
	}

	flags, err := cmd.parseTokenFlags(message_string)
	if err != nil {
		return err
	}

	request, err := getTokenRequest(flags)
	if err != nil {
		return err
	}

	token, err := server.Auth.PrepareAccessToken(request)
	if err != nil {
		return err
	}

	answer := authorization.UserTokenResponce(request, token)
	fmt.Fprintf(outBuf, "%s\n", string(answer))
	return nil
}

func getTokenRequest(flags tokenFlags) (request authorization.TokenRequest, err error) {
	switch flags.Type {
	case "user-token":
		request, err = userTokenRequest(flags)
	case "admin-token":
		request, err = adminTokenRequest(flags)
	default:
		return authorization.TokenRequest{}, errors.New("wrong token type")
	}
	if err != nil {
		return authorization.TokenRequest{}, err
	}
	return request, err
}


func (cmd *command) parseTokenFlags(message_string string) (tokenFlags, error) {

	var flags tokenFlags
	flagset := cmd.createDefaultFlagset(message_string, "")
	flagset.StringVar(&flags.Type, "type", "", "token type")
	flagset.StringVar(&flags.Beamtime, "beamtime", "", "beamtime for user token")
	flagset.StringVar(&flags.Beamline, "beamline", "", "beamline for user token")
	flagset.StringVar(&flags.AccessType, "access-type", "", "read/write for user token")
	flagset.IntVar(&flags.DaysValid, "duration-days", 0, "token duration (in days)")


	flagset.Parse(cmd.args)

	if printHelp(flagset) {
		os.Exit(0)
	}

	if flags.Type == "" {
		return flags, errors.New("secret file missed ")
	}


	return flags, nil

}
