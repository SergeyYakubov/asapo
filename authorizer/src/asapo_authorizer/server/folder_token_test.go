package server

import (
	"asapo_common/utils"
	"github.com/stretchr/testify/assert"
	"io/ioutil"
	"net/http"
	"testing"
)

var  fodlerTokenTests = [] struct {
	beamtime_id string
	token string
	status int
	message string
}{
	{"11111111", prepareToken("11111111"),http.StatusOK,"beamtime found"},
	{"11111111", prepareToken("11111112"),http.StatusUnauthorized,"wrong token"},
	{"11111111", prepareToken("11111111"),http.StatusBadRequest,"bad request"},

}

func TestFolderToken(t *testing.T) {
	for _, test := range fodlerTokenTests {
		root_folder  := "/abc/def"
		authJWT = utils.NewJWTAuth("secret")
		request :=  makeRequest(folderTokenRequest{root_folder,test.beamtime_id,test.token})
		if test.status == http.StatusBadRequest {
			request =makeRequest(authorizationRequest{})
		}
		w := doPostRequest("/folder",request)
		if test.status == http.StatusOK {
			body, _ := ioutil.ReadAll(w.Body)
			claims,_ := utils.CheckJWTToken(string(body),"secret")
			var extra_claim utils.FolderTokenTokenExtraClaim
			utils.MapToStruct(claims.(*utils.CustomClaims).ExtraClaims.(map[string]interface{}), &extra_claim)
			assert.Equal(t, root_folder, extra_claim.RootFolder, test.message)
		}
		assert.Equal(t, test.status, w.Code, test.message)
	}
}

