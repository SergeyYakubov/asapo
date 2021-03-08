package cli

import (
	"testing"

	"github.com/stretchr/testify/assert"
	"bytes"
	"io/ioutil"
	"os"
)

var tokenTests = []struct {
	cmd      command
	answer string
	msg  string
}{
	{command{args: []string{"beamtime_id"}},  "secret", "no secret parameter"},
	{command{args: []string{"-secret","secret.tmp"}},  "beamtime id", "no file"},
	{command{args: []string{"-secret","not_existing_file","beamtime_id"}},  "not_existing_file", "no file"},
	{command{args: []string{"-secret","secret.tmp","beamtime_id"}},  "eodk3s5ZXwACLGyVA63MZYcOTWuWE4bceI9Vxl9zejI=", "ok"},
}

func TestParseTokenFlags(t *testing.T) {

	ioutil.WriteFile("secret.tmp", []byte("secret"), 0644)
	outBuf = new(bytes.Buffer)
	for _, test := range tokenTests {
		err := test.cmd.CommandToken()
		if err == nil {
			assert.Contains(t, outBuf.(*bytes.Buffer).String(), test.answer, test.msg)
		} else {
			assert.Contains(t, err.Error(), test.answer, test.msg)
		}

	}
	os.Remove("secret.tmp")

}
