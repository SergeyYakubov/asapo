package cli

import (
	"asapo_common/structs"
	"asapo_common/utils"
	"asapo_tools/rest_client"
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"net/http"
	"os"
	"strings"
)

type tokenFlags struct {
	Name         string
	Endpoint     string
	AccessTypes  []string
	SecretFile   string
	TokenDetails bool
}

func generateToken(flags tokenFlags, secret string) string {
	//	hmac := utils.NewHMACAuth(secret)
	//	token,err := hmac.GenerateToken(&id)

	//	if (err!=nil) {
	//		fmt.Println(err.Error())
	//	}
	//	return token
	return ""
}

// CommandToken receives token from authorization server
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
	if err != nil {
		return err
	}

	request := structs.IssueTokenRequest{
		Subject:    map[string]string{"beamtimeId": flags.Name},
		DaysValid:  180,
		AccessTypes: flags.AccessTypes,
	}
	json_data, _ := json.Marshal(request)
	path := flags.Endpoint + "/admin/issue"

	req, err := http.NewRequest("POST", path, bytes.NewBuffer(json_data))
	if err != nil {
		return err
	}
	req.Header.Add("Content-Type", "application/json")
	req.Header.Add("Authorization", "Bearer "+secret)

	resp, err := rest_client.Client.Do(req)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return err
	}

	if resp.StatusCode != http.StatusOK {
		return errors.New("returned " + resp.Status + ": " + string(body))
	}

	if flags.TokenDetails {
		fmt.Fprintf(outBuf, "%s\n", string(body))
		return nil
	}

	var token structs.IssueTokenResponse

	err = json.Unmarshal(body, &token)
	if err == nil {
		fmt.Fprintf(outBuf, "%s\n", token.Token)
	}
	return err
}

func (cmd *command) parseTokenFlags(message_string string) (tokenFlags, error) {

	var flags tokenFlags
	flagset := cmd.createDefaultFlagset(message_string, "<token_body>")
	flagset.StringVar(&flags.SecretFile, "secret", "", "path to file with secret")
	var at string
	flagset.StringVar(&at, "type", "", "access type")
	flagset.StringVar(&flags.Endpoint, "endpoint", "", "asapo endpoint")
	flagset.BoolVar(&flags.TokenDetails, "token-details", false, "output token details")

	flagset.Parse(cmd.args)

	flags.AccessTypes = strings.Split(at,",")


	if printHelp(flagset) {
		os.Exit(0)
	}

	flags.Name = flagset.Arg(0)

	if flags.Name == "" {
		return flags, errors.New("payload missed ")
	}

	if flags.SecretFile == "" {
		return flags, errors.New("secret file missed ")
	}

	if flags.Endpoint == "" {
		return flags, errors.New("endpoint missed ")
	}

	for _,at:=range flags.AccessTypes {
		if at!="read" && at!="write" {
			return flags,errors.New("incorrect access type")
		}
	}

	return flags, nil

}
