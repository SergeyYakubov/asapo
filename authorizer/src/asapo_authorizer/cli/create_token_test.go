package cli

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/server"
	"asapo_common/structs"
	"asapo_common/utils"
	"encoding/json"
	"testing"

	"bytes"
	"github.com/stretchr/testify/assert"
)

var tokenTests = []struct {
	cmd             command
	key string
	ok              bool
	tokenAccessType string
	tokenSubject    string
	tokenExpires bool
	msg             string
}{
// good
	{command{args: []string{"-type", "user-token", "-beamtime","123","-access-type","read","-duration-days","10"}},
		"secret_user",true, "read", "bt_123", true,"user token beamtime ok"},
	{command{args: []string{"-type", "user-token", "-beamline","123","-access-type","read","-duration-days","10"}},
		"secret_user",		true, "read", "bl_123", true,"user token beamline ok"},
	{command{args: []string{"-type", "admin-token","-access-type","create"}},
		"secret_admin",true, "create", "admin", false,"admin token ok"},
// bad
	{command{args: []string{"-type", "user-token", "-beamtime","123","-access-type","create","-duration-days","10"}},
		"secret_user",false, "", "", true,"user token wrong type"},
	{command{args: []string{"-type", "user-token", "-access-type","create","-duration-days","10"}},
		"secret_user",false, "", "", true,"user token no beamtime or beamline"},
	{command{args: []string{"-type", "user-token",  "-beamtime","123","-beamline","1234", "-access-type","create","-duration-days","10"}},
		"secret_user",false, "", "", true,"user token both beamtime and beamline"},
	{command{args: []string{"-type", "admin-token","-access-type","bla"}},
		"secret_admin",false, "", "", false,"admin token wrong type"},
}

func TestGenerateToken(t *testing.T) {
	server.Auth = authorization.NewAuth(utils.NewJWTAuth("secret_user"),utils.NewJWTAuth("secret_admin"),utils.NewJWTAuth("secret"))
	for _, test := range tokenTests {
		outBuf = new(bytes.Buffer)
		err := test.cmd.CommandCreate_token()
		if !test.ok {
			assert.NotNil(t, err, test.msg)
			continue
		}
		assert.Nil(t, err, test.msg)
		var token structs.IssueTokenResponse
		json.Unmarshal(outBuf.(*bytes.Buffer).Bytes(), &token)

		claims,_ := utils.CheckJWTToken(token.Token,test.key)
		cclaims,_:= claims.(*utils.CustomClaims)
		var extra_claim structs.AccessTokenExtraClaim
		utils.MapToStruct(cclaims.ExtraClaims.(map[string]interface{}), &extra_claim)
		assert.Equal(t, test.tokenSubject, cclaims.Subject, test.msg)
		assert.Equal(t, test.tokenAccessType, extra_claim.AccessType, test.msg)
		if test.tokenExpires {
			assert.Equal(t, true, len(token.Expires)>0, test.msg)
		} else {
			assert.Empty(t, token.Expires, test.msg)
		}
	}
}
