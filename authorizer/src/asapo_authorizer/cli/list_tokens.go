package cli

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/database"
	"asapo_authorizer/server"
	"fmt"
	"os"
)

type listTokenFlags struct {
	Beamtime   string
	Beamline   string
}

func (cmd *command) CommandList_tokens() (err error) {
	message_string := "List tokens"
	if cmd.description(message_string) {
		return nil
	}

	_, err = cmd.parseTokenFlags(message_string)
	if err != nil {
		return err
	}

	var res map[string]interface{}
	_, err = db.ProcessRequest(database.Request{
		DbName:     database.KAdminDb,
		Collection: database.KTokens,
		Op:         "list_records",
	}, &res)
	if err != nil {
		return err
	}

	answer := authorization.UserTokenResponce(request, token)
	fmt.Fprintf(outBuf, "%s\n", string(answer))
	return nil
}

func (cmd *command) parseListTokenFlags(message_string string) (tokenFlags, error) {

	var flags tokenFlags
	flagset := cmd.createDefaultFlagset(message_string, "")
	flagset.StringVar(&flags.Beamtime, "beamtime", "", "beamtime for user token")
	flagset.StringVar(&flags.Beamline, "beamline", "", "beamline for user token")

	flagset.Parse(cmd.args)

	if printHelp(flagset) {
		os.Exit(0)
	}
	return flags, nil
}
