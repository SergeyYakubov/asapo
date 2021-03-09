package cli

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/server"
	"asapo_common/utils"
	"bytes"
	"testing"
	"github.com/stretchr/testify/assert"
)

var CommandTests = []struct {
	cmd    command
	ok bool
	msg string
}{
	{command{"create-token", []string{"-type", "user-token", "-beamtime","123","-access-type","read","-duration-days","1"}}, true,"ok"},
	{command{"dummy", []string{"description"}}, false,"wrong command"},
}

func TestCommand(t *testing.T) {
	outBuf = new(bytes.Buffer)
	server.Auth = authorization.NewAuth(utils.NewHMACAuth("secret"),utils.NewJWTAuth("secret_admin"),utils.NewJWTAuth("secret"))

	for _, test := range CommandTests {
		outBuf.(*bytes.Buffer).Reset()
		err := DoCommand(test.cmd.name, test.cmd.args)
		if !test.ok {
			assert.NotNil(t, err, "Should be error",test.msg)
		} else {
			assert.Nil(t, err, "Should be ok",test.msg)
		}
	}

}

func TestPrintAllCommands(t *testing.T) {
	outBuf = new(bytes.Buffer)
	PrintAllCommands()
	assert.Contains(t, outBuf.(*bytes.Buffer).String(), "token", "all commands must have token")
}
