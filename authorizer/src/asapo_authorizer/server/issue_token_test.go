package server

import (
	"asapo_common/utils"
	"fmt"
	"github.com/stretchr/testify/assert"
	"io/ioutil"
	"net/http"
	"strconv"
	"testing"
	"time"
)

var  IssueTokenTests = [] struct {
	beamtimeId string
	beamline string
	role string
	validDays string
	adminToken string
	resToken string
	status int
	message string
}{
	{"test", "","read","180",prepareToken("admin"),"aaa",http.StatusOK,"read for beamtime"},
	{"test", "","read","180",prepareToken("admin"),"aaa",http.StatusOK,"write for beamtime"},
	{"", "test","read","180",prepareToken("admin"),"aaa",http.StatusOK,"read for beamline"},
	{"test", "test","read","180",prepareToken("bla"),"",http.StatusBadRequest,"both beamline/beamtime given"},
	{"", "","read","180",prepareToken("bla"),"",http.StatusBadRequest,"beamline or beamtime not given"},
	{"test", "","bla","180",prepareToken("bla"),"",http.StatusBadRequest,"wrong role"},
	{"test", "","read","aaa",prepareToken("bla"),"",http.StatusBadRequest,"wrong duration"},
	{"test", "","read","180",prepareToken("bla"),"",http.StatusUnauthorized,"wrong admin token"},
}

func TestIssueToken(t *testing.T) {
	for _, test := range IssueTokenTests {
		authJWT = utils.NewJWTAuth("secret")
		path := "/admin/issue"+"?beamtime="+test.beamtimeId+"&beamline="+test.beamline+"&valid="+test.validDays+"&role="+test.role
		w := doGetRequest(path,test.adminToken)
		if w.Code == http.StatusOK {
			body, _ := ioutil.ReadAll(w.Body)
			claims,_ := utils.CheckJWTToken(string(body),"secret")
			cclaims,_:= claims.(*utils.CustomClaims)
			var extra_claim utils.AccessTokenExtraClaim
			utils.MapToStruct(claims.(*utils.CustomClaims).ExtraClaims.(map[string]interface{}), &extra_claim)
			assert.Equal(t, cclaims.Subject , test.beamtimeId+test.beamline, test.message)
			day,_:=strconv.Atoi(test.validDays)
			assert.Equal(t, cclaims.Duration , time.Duration(24*day)*time.Hour, test.message)
			assert.Equal(t, extra_claim.Role , test.role, test.message)
		} else {
			body, _ := ioutil.ReadAll(w.Body)
			fmt.Println(string(body))
		}

		assert.Equal(t, test.status, w.Code, test.message)
	}
}

