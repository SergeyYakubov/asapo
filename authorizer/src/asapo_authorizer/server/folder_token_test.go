package server

import (
	"asapo_authorizer/authorization"
	"asapo_common/structs"
	"asapo_common/utils"
	"github.com/stretchr/testify/assert"
	"io/ioutil"
	"net/http"
	"testing"
	"os"
	"path/filepath"
	"fmt"
)

var  fodlerTokenTests = [] struct {
	beamtime_id string
	root_folder string
	token string
	status int
	message string
}{
	{"test", "tf/gpfs/bl1/2019/data/test", prepareUserToken("bt_test",[]string{"read"}),http.StatusOK,"beamtime found"},
	{"test_online", "bl1/current", prepareUserToken("bt_test_online",[]string{"read"}),http.StatusOK,"online beamtime found"},
	{"test", "bl1/current", prepareUserToken("bt_test",[]string{"read"}),http.StatusUnauthorized,"no online beamtime found"},
	{"test_online", "bl2/current", prepareUserToken("bt_test_online",[]string{"read"}),http.StatusUnauthorized,"wrong online folder"},
	{"test", "tf/gpfs/bl1/2019/data/test1", prepareUserToken("bt_test",[]string{"read"}),http.StatusUnauthorized,"wrong folder"},
	{"test", "tf/gpfs/bl1/2019/data/test", prepareUserToken("bt_test1",[]string{"read"}),http.StatusUnauthorized,"wrong token"},
	{"11111111", "tf/gpfs/bl1/2019/data/test", prepareUserToken("bt_11111111",[]string{"read"}),http.StatusBadRequest,"bad request"},
}

func TestFolderToken(t *testing.T) {
	allowBeamlines([]beamtimeMeta{})
	settings.RootBeamtimesFolder ="."
	settings.CurrentBeamlinesFolder="."
	Auth = authorization.NewAuth(utils.NewJWTAuth("secret_user"),utils.NewJWTAuth("secret_admin"),utils.NewJWTAuth("secret_folder"))

	os.MkdirAll(filepath.Clean("tf/gpfs/bl1/2019/data/test"), os.ModePerm)
	os.MkdirAll(filepath.Clean("tf/gpfs/bl1/2019/data/test_online"), os.ModePerm)

	os.MkdirAll(filepath.Clean("bl1/current"), os.ModePerm)
	ioutil.WriteFile(filepath.Clean("bl1/current/beamtime-metadata-test_online.json"), []byte(beamtime_meta_online), 0644)

	defer 	os.RemoveAll("tf")
	defer 	os.RemoveAll("bl1")

	for _, test := range fodlerTokenTests {
		abs_path:=settings.RootBeamtimesFolder + string(filepath.Separator)+test.root_folder
		request :=  makeRequest(folderTokenRequest{abs_path,test.beamtime_id,test.token})
		if test.status == http.StatusBadRequest {
			request =makeRequest(authorizationRequest{})
		}
		w := doPostRequest("/v0.1/folder",request,"")
		if w.Code == http.StatusOK {
			body, _ := ioutil.ReadAll(w.Body)
			claims,_ := utils.CheckJWTToken(string(body),"secret_folder")
			var extra_claim structs.FolderTokenTokenExtraClaim
			utils.MapToStruct(claims.(*utils.CustomClaims).ExtraClaims.(map[string]interface{}), &extra_claim)
			assert.Equal(t, abs_path, extra_claim.RootFolder, test.message)
		} else {
			body, _ := ioutil.ReadAll(w.Body)
			fmt.Println(string(body))
		}

		assert.Equal(t, test.status, w.Code, test.message)
	}
}

func TestFolderTokenWrongProtocol(t *testing.T) {
		request :=  makeRequest(folderTokenRequest{"abs_path","beamtime_id","token"})
		w := doPostRequest("/v0.2/folder",request,"")
		assert.Equal(t, http.StatusUnsupportedMediaType, w.Code, "wrong protocol")
}



