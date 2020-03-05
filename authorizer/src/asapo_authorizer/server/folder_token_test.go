package server

import (
	"asapo_common/utils"
	"encoding/json"
	"fmt"
	"github.com/stretchr/testify/assert"
	"io/ioutil"
	"net/http"
	"testing"
)



func TestFolderTokenOK(t *testing.T) {
	root_folder  := "/abc/def"
	settings.secret = "secret"
	request :=  makeRequest(folderTokenRequest{root_folder,"11111111",prepareToken("11111111")})
	w := doPostRequest("/folder",request)

	body, _ := ioutil.ReadAll(w.Body)
	var jwt_token  folderToken
	json.Unmarshal(body, &jwt_token)


	claims,ok := utils.CheckJWTToken(jwt_token.Token,"secret")
	fmt.Println(claims,ok)
	var extra_claim TokenExtraClaim
	utils.MapToStruct(claims.(*utils.CustomClaims).ExtraClaims.(map[string]interface{}), &extra_claim)

	assert.Equal(t, root_folder, extra_claim.RootFolder, "")
	assert.Equal(t, http.StatusOK, w.Code, "")
}

