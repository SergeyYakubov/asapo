package cli

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/database"
	"asapo_authorizer/server"
	"asapo_common/structs"
	"errors"
	"fmt"
	"os"
	"strings"
)

type tokenFlags struct {
	Type       string
	AccessType string
	Beamtime   string
	Beamline   string
	DaysValid  int
}

func userTokenRequest(flags tokenFlags) (request structs.IssueTokenRequest, err error) {
	if (flags.Beamline == "" && flags.Beamtime == "") || (flags.Beamline != "" && flags.Beamtime != "") {
		return request, errors.New("beamtime or beamline must be set")
	}

	request.Subject = make(map[string]string, 1)
	if flags.Beamline != "" {
		request.Subject["beamline"] = flags.Beamline
	} else {
		request.Subject["beamtimeId"] = flags.Beamtime
	}

	request.AccessTypes = strings.Split(flags.AccessType, ",")
	for _, at := range request.AccessTypes {
		if at != "read" && at != "write" && !(at == "writeraw" && request.Subject["beamline"] != "") {
			if request.Subject["beamline"] != "" {
				return request, errors.New("access type must be read, write or writeraw")
			} else {
				return request, errors.New("access type must be read or write")
			}

		}
	}

	request.DaysValid = flags.DaysValid

	return
}

func adminTokenRequest(flags tokenFlags) (request structs.IssueTokenRequest, err error) {
	if flags.Beamline+flags.Beamtime != "" {
		return request, errors.New("beamtime and beamline must not be set for admin token")
	}

	request.AccessTypes = strings.Split(flags.AccessType, ",")
	for _, at := range request.AccessTypes {
		if at != "create" && at != "revoke" && at != "list" {
			return request, errors.New("access type must be create,revoke of list")
		}
	}

	request.Subject = make(map[string]string, 1)
	request.Subject["user"] = "admin"
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

	request, userToken, err := getTokenRequest(flags)
	if err != nil {
		return err
	}

	token, claims, err := server.Auth.PrepareAccessToken(request, userToken)
	if err != nil {
		return err
	}

	record := database.TokenRecord{claims.Id, claims, token}
	_, err = db.ProcessRequest(database.Request{
		DbName:     database.KAdminDb,
		Collection: database.KTokens,
		Op:         "create_record",
	}, &record)
	if err != nil {
		return err
	}

	answer := authorization.UserTokenResponce(request, token)
	fmt.Fprintf(outBuf, "%s\n", string(answer))
	return nil
}

func getTokenRequest(flags tokenFlags) (request structs.IssueTokenRequest, userToken bool, err error) {
	switch flags.Type {
	case "user-token":
		request, err = userTokenRequest(flags)
		userToken = true
	case "admin-token":
		request, err = adminTokenRequest(flags)
		userToken = false
	default:
		return structs.IssueTokenRequest{}, false, errors.New("wrong token type")
	}
	if err != nil {
		return structs.IssueTokenRequest{}, false, err
	}
	return request, userToken, err
}

func (cmd *command) parseTokenFlags(message_string string) (tokenFlags, error) {

	var flags tokenFlags
	flagset := cmd.createDefaultFlagset(message_string, "")
	flagset.StringVar(&flags.Type, "type", "", "token type")
	flagset.StringVar(&flags.Beamtime, "beamtime", "", "beamtime for user token")
	flagset.StringVar(&flags.Beamline, "beamline", "", "beamline for user token")
	flagset.StringVar(&flags.AccessType, "access-types", "", "read/write/writeraw(beamline only) for user token")
	flagset.IntVar(&flags.DaysValid, "duration-days", 0, "token duration (in days)")

	flagset.Parse(cmd.args)

	if printHelp(flagset) {
		os.Exit(0)
	}

	if flags.Type == "" {
		return flags, errors.New("access types missing")
	}

	return flags, nil

}
