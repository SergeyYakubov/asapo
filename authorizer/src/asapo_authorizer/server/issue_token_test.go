package server

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/token_store"
	"asapo_common/structs"
	"asapo_common/utils"
	"encoding/json"
	"fmt"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"io/ioutil"
	"net/http"
	"testing"
	"time"
)

var  IssueTokenTests = [] struct {
	requestSubject map[string]string
	tokenSubject   string
	roles          []string
	validDays      int
	adminToken     string
	resToken       string
	status         int
	message        string
}{
	{map[string]string{"beamtimeId":"test"},"bt_test",[]string{"read"},180,prepareAdminToken("admin"),"aaa",http.StatusOK,"read for beamtime"},
	{map[string]string{"beamtimeId":"test"},"bt_test",[]string{"write"},90,prepareAdminToken("admin"),"aaa",http.StatusOK,"write for beamtime"},
	{map[string]string{"beamtimeId":"test"},"bt_test",[]string{"writeraw"},90,prepareAdminToken("admin"),"",http.StatusBadRequest,"wrong role"},
	{map[string]string{"beamline":"test"},"bl_test",[]string{"writeraw"},90,prepareAdminToken("admin"),"aaa",http.StatusOK,"raw for beamline"},
	{map[string]string{"beamline":"test"},"bl_test",[]string{"read"},180,prepareAdminToken("admin"),"aaa",http.StatusOK,"read for beamline"},
	{map[string]string{"blabla":"test"},"",[]string{"read"},180,prepareAdminToken("admin"),"",http.StatusBadRequest,"beamline or beamtime not given"},
	{map[string]string{"beamtimeId":"test"},"",[]string{"bla"},180,prepareAdminToken("admin"),"",http.StatusBadRequest,"wrong role"},
	{map[string]string{"beamtimeId":"test"},"",[]string{"read"},180,prepareAdminToken("bla"),"",http.StatusUnauthorized,"wrong admin token"},
	{map[string]string{"beamtimeId":"test"},"bt_test",[]string{"read"},0,prepareAdminToken("admin"),"aaa",http.StatusBadRequest,"0 valid days"},

}

func TestIssueToken(t *testing.T) {
	authJWT := utils.NewJWTAuth("secret")
	authAdmin := utils.NewJWTAuth("secret_admin")
	authUser := utils.NewJWTAuth("secret_user")
	Auth = authorization.NewAuth(authUser,authAdmin,authJWT)
	mock_store := new(token_store.MockedStore)
	store = mock_store

	for _, test := range IssueTokenTests {
		request :=  makeRequest(structs.IssueTokenRequest{test.requestSubject,test.validDays,test.roles})
		mock_store.On("IsTokenRevoked", mock.Anything).Return(false,nil)
		if test.status == http.StatusOK {
			mock_store.On("AddToken", mock.Anything).Return(nil)
		}
		w := doPostRequest("/admin/issue",request,authAdmin.Name()+" "+test.adminToken)
		if w.Code == http.StatusOK {
			body, _ := ioutil.ReadAll(w.Body)
			var token structs.IssueTokenResponse
			json.Unmarshal(body,&token)
			claims,_ := utils.CheckJWTToken(token.Token,"secret_user")
			cclaims,_:= claims.(*utils.CustomClaims)
			var extra_claim structs.AccessTokenExtraClaim
			utils.MapToStruct(claims.(*utils.CustomClaims).ExtraClaims.(map[string]interface{}), &extra_claim)
			assert.Equal(t, cclaims.Subject , test.tokenSubject, test.message)
			assert.True(t, cclaims.ExpiresAt-time.Now().Unix()>int64(test.validDays)*24*60*60-10, test.message)
			assert.True(t, cclaims.ExpiresAt-time.Now().Unix()<int64(test.validDays)*24*60*60+10, test.message)
			assert.Equal(t, extra_claim.AccessTypes, test.roles, test.message)
			assert.NotEmpty(t, cclaims.Id , test.message)
		} else {
			body, _ := ioutil.ReadAll(w.Body)
			fmt.Println(string(body))
		}
		mock_store.AssertExpectations(t)
		mock_store.ExpectedCalls = nil
		mock_store.Calls = nil

		assert.Equal(t, test.status, w.Code, test.message)
	}
}

