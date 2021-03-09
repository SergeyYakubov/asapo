package server

import (
	"asapo_authorizer/authorization"
	"asapo_common/utils"
	"encoding/json"
	"fmt"
	"github.com/stretchr/testify/assert"
	"io/ioutil"
	"net/http"
	"testing"
	"time"
)

var  IssueTokenTests = [] struct {
	requestSubject map[string]string
	tokenSubject string
	role string
	validDays int
	adminToken string
	resToken string
	status int
	message string
}{
	{map[string]string{"beamtimeId":"test"},"bt_test","read",180,prepareAdminToken("admin"),"aaa",http.StatusOK,"read for beamtime"},
	{map[string]string{"beamtimeId":"test"},"bt_test","read",90,prepareAdminToken("admin"),"aaa",http.StatusOK,"write for beamtime"},
	{map[string]string{"beamline":"test"},"bl_test","read",180,prepareAdminToken("admin"),"aaa",http.StatusOK,"read for beamline"},
	{map[string]string{"blabla":"test"},"","read",180,prepareAdminToken("admin"),"",http.StatusBadRequest,"beamline or beamtime not given"},
	{map[string]string{"beamtimeId":"test"},"","bla",180,prepareAdminToken("admin"),"",http.StatusBadRequest,"wrong role"},
	{map[string]string{"beamtimeId":"test"},"","read",180,prepareAdminToken("bla"),"",http.StatusUnauthorized,"wrong admin token"},
}

func TestIssueToken(t *testing.T) {
	authJWT := utils.NewJWTAuth("secret")
	authAdmin := utils.NewJWTAuth("secret_admin")
	Auth = authorization.NewAuth(nil,authAdmin,authJWT)
	for _, test := range IssueTokenTests {
		request :=  makeRequest(authorization.TokenRequest{test.requestSubject,test.validDays,test.role})
		w := doPostRequest("/admin/issue",request,authAdmin.Name()+" "+test.adminToken)
		if w.Code == http.StatusOK {
			body, _ := ioutil.ReadAll(w.Body)
			var token authorization.TokenResponce
			json.Unmarshal(body,&token)
			claims,_ := utils.CheckJWTToken(token.Token,"secret_admin")
			cclaims,_:= claims.(*utils.CustomClaims)
			var extra_claim utils.AccessTokenExtraClaim
			utils.MapToStruct(claims.(*utils.CustomClaims).ExtraClaims.(map[string]interface{}), &extra_claim)
			assert.Equal(t, cclaims.Subject , test.tokenSubject, test.message)
			assert.True(t, cclaims.ExpiresAt-time.Now().Unix()>int64(test.validDays)*24*60*60-10, test.message)
			assert.True(t, cclaims.ExpiresAt-time.Now().Unix()<int64(test.validDays)*24*60*60+10, test.message)
			assert.Equal(t, extra_claim.AccessType, test.role, test.message)
			assert.NotEmpty(t, cclaims.Id , test.message)
		} else {
			body, _ := ioutil.ReadAll(w.Body)
			fmt.Println(string(body))
		}

		assert.Equal(t, test.status, w.Code, test.message)
	}
}

