package cli

import (
	"bytes"
	"testing"
	"github.com/stretchr/testify/assert"
)

var CommandTests = []struct {
	cmd    command
	answer string
}{
	{command{"token", []string{"-secret", "secret_file","-type","read","-endpoint","bla", "beamtime"}}, "secret"},
	{command{"dummy", []string{"description"}}, "wrong"},
}

func TestCommand(t *testing.T) {
	outBuf = new(bytes.Buffer)

	for _, test := range CommandTests {
		outBuf.(*bytes.Buffer).Reset()
		err := DoCommand(test.cmd.name, test.cmd.args)
		assert.Contains(t, err.Error(), test.answer, "")
		assert.NotNil(t, err, "Should be error")
	}

}

func TestPrintAllCommands(t *testing.T) {
	outBuf = new(bytes.Buffer)
	PrintAllCommands()
	assert.Contains(t, outBuf.(*bytes.Buffer).String(), "token", "all commands must have token")
}
