package cli

import (
	"asapo_tools/mocks"
	"asapo_tools/rest_client"
	"encoding/json"
	"fmt"
	"net/http"
	"testing"

	"bytes"
	"github.com/stretchr/testify/assert"
	"io/ioutil"
	"os"
)

var tokenTests = []struct {
	cmd      command
	ok bool
	msg  string
}{
	{command{args: []string{"beamtime_id"}},  false, "no secret parameter"},
	{command{args: []string{"-secret","secret.tmp"}},  false, "no file"},
	{command{args: []string{"-secret","not_existing_file","payload"}},  false, "no file"},
	{command{args: []string{"-secret","secret.tmp","beamtime_id"}},  false, "type is missing"},
	{command{args: []string{"-secret","secret.tmp","-type","read","beamtime_id"}},  false, "endpoint is missing"},
	{command{args: []string{"-secret","secret.tmp","-type","read","-endpoint","endpoint","-token-details","beamtime_id"}},  true, "ok"},
}

func TestParseTokenFlags(t *testing.T) {

	ioutil.WriteFile("secret.tmp", []byte("secret"), 0644)
	outBuf = new(bytes.Buffer)

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
		err := test.cmd.CommandToken()
		if test.ok {
			assert.Nil(t, err, test.msg)
			resp := struct {
				Token string
				Uri string
			}{}
			err := json.Unmarshal(outBuf.(*bytes.Buffer).Bytes(),&resp)
			fmt.Println(err)
			assert.Equal(t,  "blabla", resp.Token,test.msg)
			assert.Equal(t, "endpoint/admin/issue",resp.Uri, test.msg)
		} else {
			assert.NotNil(t, err, test.msg)
		}

	}
	os.Remove("secret.tmp")

}
