package cli

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/server"
	"asapo_authorizer/token_store"
	"asapo_common/structs"
	"asapo_common/utils"
	"encoding/json"
	"github.com/stretchr/testify/mock"
	"testing"

	"bytes"
	"github.com/stretchr/testify/assert"
)

var tokenTests = []struct {
	cmd             command
	key string
	ok              bool
	tokenAccessTypes []string
	tokenSubject    string
	tokenExpires bool
	msg             string
}{
// good
	{command{args: []string{"-type", "user-token", "-beamtime","123","-access-types","read","-duration-days","10"}},
		"secret_user",true, []string{"read"}, "bt_123", true,"user token beamtime ok"},
	{command{args: []string{"-type", "user-token", "-beamline","123","-access-types","read","-duration-days","10"}},
		"secret_user",		true, []string{"read"}, "bl_123", true,"user token beamline ok"},
	{command{args: []string{"-type", "admin-token","-access-types","create"}},
		"secret_admin",true, []string{"create"}, "admin", false,"admin token ok"},
// bad
	{command{args: []string{"-type", "user-token", "-beamtime","123","-access-types","create","-duration-days","10"}},
		"secret_user",false, nil, "", true,"user token wrong type"},
	{command{args: []string{"-type", "user-token", "-access-types","create","-duration-days","10"}},
		"secret_user",false, nil, "", true,"user token no beamtime or beamline"},
	{command{args: []string{"-type", "user-token",  "-beamtime","123","-beamline","1234", "-access-types","create","-duration-days","10"}},
		"secret_user",false, nil, "", true,"user token both beamtime and beamline"},
	{command{args: []string{"-type", "admin-token","-access-types","bla"}},
		"secret_admin",false, nil ,"", false,"admin token wrong type"},
}

func TestGenerateToken(t *testing.T) {
	server.Auth = authorization.NewAuth(utils.NewJWTAuth("secret_user"),utils.NewJWTAuth("secret_admin"),utils.NewJWTAuth("secret"))
	mock_store := new(token_store.MockedStore)
	store = mock_store

	for _, test := range tokenTests {
		outBuf = new(bytes.Buffer)

		if test.ok {
			mock_store.On("AddToken", mock.Anything).Return(nil)
		}

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
		assert.Equal(t, test.tokenAccessTypes, extra_claim.AccessTypes, test.msg)
		if test.tokenExpires {
			assert.Equal(t, true, len(token.Expires)>0, test.msg)
		} else {
			assert.Empty(t, token.Expires, test.msg)
		}

		mock_store.AssertExpectations(t)
		mock_store.ExpectedCalls = nil
		mock_store.Calls = nil
	}
}
