package cli

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/server"
	"asapo_common/utils"
	"bytes"
	"github.com/stretchr/testify/assert"
	"testing"
)

var CommandTests = []struct {
	cmd   command
	error string
	msg   string
}{
	{command{"create-token", []string{"-type", "user-token", "-beamtime", "123", "-access-types", "read", "-duration-days", "1"}}, "database", "ok"},
	{command{"list-tokens", []string{}}, "database", "ok"},
	{command{"revoke-token", []string{"-token","123"}}, "database", "ok"},
	{command{"revoke-token", []string{"-token-id","123"}}, "database", "ok"},
	{command{"dummy", []string{"description"}}, "wrong", "wrong command"},
}

func TestCommand(t *testing.T) {
	outBuf = new(bytes.Buffer)

	server.Auth = authorization.NewAuth(utils.NewJWTAuth("secret"), utils.NewJWTAuth("secret_admin"), utils.NewJWTAuth("secret"))
	for _, test := range CommandTests {
		outBuf.(*bytes.Buffer).Reset()
		err := DoCommand(test.cmd.name, test.cmd.args)
		if err != nil {
			assert.Contains(t, err.Error(), test.error)
		}
	}

}

func TestPrintAllCommands(t *testing.T) {
	outBuf = new(bytes.Buffer)
	PrintAllCommands()
	assert.Contains(t, outBuf.(*bytes.Buffer).String(), "token", "all commands must have token")
}
