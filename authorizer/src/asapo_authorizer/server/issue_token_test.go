package server

import (
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
	beamtimeId string
	beamline string
	subject string
	role string
	validDays int
	adminToken string
	resToken string
	status int
	message string
}{
	{"test", "","bt_test","read",180,prepareToken("admin"),"aaa",http.StatusOK,"read for beamtime"},
	{"test", "","bt_test","read",90,prepareToken("admin"),"aaa",http.StatusOK,"write for beamtime"},
	{"", "test","bl_test","read",180,prepareToken("admin"),"aaa",http.StatusOK,"read for beamline"},
	{"test", "test","","read",180,prepareToken("admin"),"",http.StatusBadRequest,"both beamline/beamtime given"},
	{"", "","","read",180,prepareToken("admin"),"",http.StatusBadRequest,"beamline or beamtime not given"},
	{"test", "","","bla",180,prepareToken("admin"),"",http.StatusBadRequest,"wrong role"},
	{"test", "","","read",180,prepareToken("bla"),"",http.StatusUnauthorized,"wrong admin token"},
}

func TestIssueToken(t *testing.T) {
	authJWT = utils.NewJWTAuth("secret")
	authHMAC = utils.NewHMACAuth("secret")
	for _, test := range IssueTokenTests {
		request :=  makeRequest(userTokenRequest{test.beamtimeId,test.beamline,test.validDays,test.role})
		w := doPostRequest("/admin/issue",request,authHMAC.Name()+" "+test.adminToken)
		if w.Code == http.StatusOK {
			body, _ := ioutil.ReadAll(w.Body)
			var token userToken
			json.Unmarshal(body,&token)
			claims,_ := utils.CheckJWTToken(token.Token,"secret")
			cclaims,_:= claims.(*utils.CustomClaims)
			var extra_claim utils.AccessTokenExtraClaim
			utils.MapToStruct(claims.(*utils.CustomClaims).ExtraClaims.(map[string]interface{}), &extra_claim)
			assert.Equal(t, cclaims.Subject , test.subject, test.message)
			assert.True(t, cclaims.ExpiresAt-time.Now().Unix()>int64(test.validDays)*24*60*60-10, test.message)
			assert.True(t, cclaims.ExpiresAt-time.Now().Unix()<int64(test.validDays)*24*60*60+10, test.message)
			assert.Equal(t, extra_claim.Role , test.role, test.message)
			assert.NotEmpty(t, cclaims.Id , test.message)
		} else {
			body, _ := ioutil.ReadAll(w.Body)
			fmt.Println(string(body))
		}

		assert.Equal(t, test.status, w.Code, test.message)
	}
}

