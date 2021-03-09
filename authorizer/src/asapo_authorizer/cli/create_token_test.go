package cli

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/server"
	"asapo_common/utils"
	"encoding/json"
	"fmt"
	"testing"

	"bytes"
	"github.com/stretchr/testify/assert"
)

var tokenTests = []struct {
	cmd             command
	ok              bool
	tokenAccessType string
	tokenSubject    string
	tokenExpires bool
	msg             string
}{
//	{command{args: []string{"-type"}}, false, "", "", "not enough parameters"},
//	{command{args: []string{"-type", "user-token", "-beamtime","123","-access-type","read","-duration-days","10"}},
//		true, "read", "bt_123", true,"user token ok"},
	{command{args: []string{"-type", "admin-token","-access-type","create"}},
		true, "create", "admin", false,"admin token ok"},
}

func TestGenerateToken(t *testing.T) {
	outBuf = new(bytes.Buffer)
	server.Auth = authorization.NewAuth(utils.NewHMACAuth("secret"),utils.NewHMACAuth("secret"),utils.NewJWTAuth("secret"))
	for _, test := range tokenTests {
		err := test.cmd.CommandCreate_token()
		fmt.Println(err)
		if !test.ok {
			assert.NotNil(t, err, test.msg)
			continue
		}
		assert.Nil(t, err, test.msg)
		var token authorization.TokenResponce
		json.Unmarshal(outBuf.(*bytes.Buffer).Bytes(), &token)
		assert.Equal(t, test.tokenSubject, token.Sub, test.msg)
		assert.Equal(t, test.tokenAccessType, token.AccessType, test.msg)
		if test.tokenExpires {
			assert.Equal(t, true, len(token.Expires)>0, test.msg)
		} else {
			assert.Empty(t, token.Expires, test.msg)
		}

	}
}
