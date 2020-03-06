package server

import (
	"asapo_common/utils"
	"encoding/json"
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
}

func TestFolderToken(t *testing.T) {
	for _, test := range fodlerTokenTests {
		root_folder  := "/abc/def"
		authJWT = utils.NewJWTAuth("secret")
		request :=  makeRequest(folderTokenRequest{root_folder,test.beamtime_id,test.token})
		w := doPostRequest("/folder",request)
		if test.status == http.StatusOK {
			body, _ := ioutil.ReadAll(w.Body)
			var jwt_token  folderToken
			json.Unmarshal(body, &jwt_token)
			_,token,_:=utils.SplitAuthToken(jwt_token.Token)
			claims,_ := utils.CheckJWTToken(token,"secret")
			var extra_claim utils.FolderTokenTokenExtraClaim
			utils.MapToStruct(claims.(*utils.CustomClaims).ExtraClaims.(map[string]interface{}), &extra_claim)
			assert.Equal(t, root_folder, extra_claim.RootFolder, test.message)
		}
		assert.Equal(t, test.status, w.Code, test.message)
	}
}

