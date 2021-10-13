package cli

import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
)

type revokeTokenFlags struct {
	Token   string
	TokenId string
}

func (cmd *command) CommandRevoke_token() (err error) {
	message_string := "Revoke token"
	if cmd.description(message_string) {
		return nil
	}

	flags, err := cmd.parseRevokeTokenFlags(message_string)
	if err != nil {
		return err
	}
	token,err := store.RevokeToken(flags.Token, flags.TokenId)
	if err != nil {
		return err
	}

	out, _ := json.Marshal(token)
	fmt.Fprintln(outBuf, string(out))
	return nil
}

func (cmd *command) parseRevokeTokenFlags(message_string string) (revokeTokenFlags, error) {

	var flags revokeTokenFlags
	flagset := cmd.createDefaultFlagset(message_string, "")
	flagset.StringVar(&flags.Token, "token", "", "token to revoke")
	flagset.StringVar(&flags.TokenId, "token-id", "", "token id to revoke")
	flagset.Parse(cmd.args)

	if printHelp(flagset) {
		os.Exit(0)
	}

	if flags.Token == "" && flags.TokenId == "" {
		return flags, errors.New("set token or token id to revoke")
	}

	if flags.Token != "" && flags.TokenId != "" {
		return flags, errors.New("cannot use both token and token id")
	}

	return flags, nil

}
