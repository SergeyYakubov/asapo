package cli

import (
	"errors"
	"os"
	"fmt"
	"asapo_common/utils"
)

type tokenFlags struct {
	BeamtimeID string
	SecretFile string
}

func generateToken(id string,secret string) string {
	hmac := utils.NewHMACAuth(secret)
	token,err := hmac.GenerateToken(&id)

	if (err!=nil) {
		fmt.Println(err.Error())
	}
	return token
}


// GenerateToken generates token for consumers
func (cmd *command) CommandToken() error {

	message_string := "Generate token"

	if cmd.description(message_string) {
		return nil
	}

	flags, err := cmd.parseTokenFlags(message_string)
	if err != nil {
		return err
	}

	secret, err := utils.ReadFirstStringFromFile(flags.SecretFile)
	if err !=nil  {
		return err
	}

	fmt.Fprintf(outBuf, "%s\n", generateToken(flags.BeamtimeID,secret))

	return nil
}


func (cmd *command) parseTokenFlags(message_string string) (tokenFlags, error) {

	var flags tokenFlags
	flagset := cmd.createDefaultFlagset(message_string, "<beamtime id>")
	flagset.StringVar(&flags.SecretFile, "secret", "", "path to file with secret")

	flagset.Parse(cmd.args)

	if printHelp(flagset) {
		os.Exit(0)
	}

	flags.BeamtimeID = flagset.Arg(0)

	if flags.BeamtimeID == "" {
		return flags, errors.New("beamtime id missed ")
	}

	if flags.SecretFile == "" {
		return flags, errors.New("secret file missed ")
	}


	return flags, nil

}
