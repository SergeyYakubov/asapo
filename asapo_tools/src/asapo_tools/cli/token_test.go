package cli

import (
	"asapo_tools/mocks"
	"asapo_tools/rest_client"
	"encoding/json"
	"net/http"
	"testing"

	"bytes"
	"github.com/stretchr/testify/assert"
	"io/ioutil"
	"os"
)

var tokenTests = []struct {
	cmd      command
	withDetails bool
	ok bool
	msg  string
}{
	{command{args: []string{"beamtime_id"}},  false,false, "no secret parameter"},
	{command{args: []string{"-secret","secret.tmp"}},  false,false, "no file"},
	{command{args: []string{"-secret","not_existing_file","payload"}}, false, false, "no file"},
	{command{args: []string{"-secret","secret.tmp","beamtime_id"}},false,  false, "type is missing"},
	{command{args: []string{"-secret","secret.tmp","-type","read","beamtime_id"}}, false, false, "endpoint is missing"},
	{command{args: []string{"-secret","secret.tmp","-type","read","-endpoint","endpoint","-token-details","beamtime_id"}},true,  true, "ok"},
	{command{args: []string{"-secret","secret.tmp","-type","read","-endpoint","endpoint","beamtime_id"}},  false,true, "without details"},
}

func TestParseTokenFlags(t *testing.T) {

	ioutil.WriteFile("secret.tmp", []byte("secret"), 0644)

	rest_client.Client = &mocks.MockClient{}


	mocks.DoFunc = func(req *http.Request) (*http.Response, error) {
		json := `{"Token":"blabla","Uri":"`+req.URL.Path+`"}`
		r := ioutil.NopCloser(bytes.NewReader([]byte(json)))

		return &http.Response{
			StatusCode: 200,
			Body:       r,
		}, nil
	}

	for _, test := range tokenTests {
		outBuf = new(bytes.Buffer)
		err := test.cmd.CommandToken()
		if test.ok {
			assert.Nil(t, err, test.msg)
			resp := struct {
				Token string
				Uri string
			}{}
			if test.withDetails {
				err := json.Unmarshal(outBuf.(*bytes.Buffer).Bytes(),&resp)
				assert.Nil(t, err, test.msg)
				assert.Equal(t,  "blabla", resp.Token,test.msg)
				assert.Equal(t, "endpoint/admin/issue",resp.Uri, test.msg)
			} else {
				assert.Equal(t,  "blabla\n", outBuf.(*bytes.Buffer).String(),test.msg)
			}
		} else {
			assert.NotNil(t, err, test.msg)
		}

	}
	os.Remove("secret.tmp")

}
