package server

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/common"
	"asapo_authorizer/token_store"
	"asapo_common/structs"
	"asapo_common/utils"
	"asapo_common/version"
	"fmt"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"io/ioutil"
	"net/http"
	"os"
	"path/filepath"
	"testing"
)

var  folderTokenTests = [] struct {
	beamtime_id   string
	auto          bool
	root_folder   string
	second_folder string
	token         string
	status        int
	message       string
}{
	{"test", false,"tf/gpfs/bl1/2019/data/test", "", prepareAsapoToken("bt_test",[]string{"read"}),http.StatusOK,"beamtime found"},
/*	{"test_online",false, "bl1/current", "", prepareAsapoToken("bt_test_online",[]string{"read"}),http.StatusOK,"online beamtime found"},
	{"test", false,"bl1/current", "", prepareAsapoToken("bt_test",[]string{"read"}),http.StatusUnauthorized,"no online beamtime found"},
	{"test_online",false, "bl2/current", "", prepareAsapoToken("bt_test_online",[]string{"read"}),http.StatusUnauthorized,"wrong online folder"},
	{"test", false,"tf/gpfs/bl1/2019/data/test1", "", prepareAsapoToken("bt_test",[]string{"read"}),http.StatusUnauthorized,"wrong folder"},
	{"test", false,"tf/gpfs/bl1/2019/data/test", "", prepareAsapoToken("bt_test1",[]string{"read"}),http.StatusUnauthorized,"wrong token"},
	{"11111111", false,"tf/gpfs/bl1/2019/data/test", "", prepareAsapoToken("bt_11111111",[]string{"read"}),http.StatusBadRequest,"bad request"},

	{"test", true,"tf/gpfs/bl1/2019/data/test", "", prepareAsapoToken("bt_test",[]string{"read"}),http.StatusOK,"auto without onilne"},
	{"test_online",true, "tf/gpfs/bl1/2019/data/test_online", "bl1/current", prepareAsapoToken("bt_test_online",[]string{"read"}),http.StatusOK,"auto with online"},
*/
}

func TestFolderToken(t *testing.T) {
	allowBeamlines([]common.BeamtimeMeta{})
	mock_store := new(token_store.MockedStore)
	store = mock_store

	common.Settings.RootBeamtimesFolder ="."
	common.Settings.CurrentBeamlinesFolder="."
	Auth = authorization.NewAuth(utils.NewJWTAuth("secret_user"),utils.NewJWTAuth("secret_admin"),utils.NewJWTAuth("secret_folder"))

	os.MkdirAll(filepath.Clean("tf/gpfs/bl1/2019/data/test"), os.ModePerm)
	os.MkdirAll(filepath.Clean("tf/gpfs/bl1/2019/data/test_online"), os.ModePerm)

	os.MkdirAll(filepath.Clean("bl1/current"), os.ModePerm)
	ioutil.WriteFile(filepath.Clean("bl1/current/beamtime-metadata-test_online.json"), []byte(beamtime_meta_online), 0644)

	defer 	os.RemoveAll("tf")
	defer 	os.RemoveAll("bl1")

	for _, test := range folderTokenTests {
		testName := "Testcase " + test.message

		abs_path := common.Settings.RootBeamtimesFolder + string(filepath.Separator)+test.root_folder
		abs_path_second :=""
		if test.second_folder!="" {
			abs_path_second =common.Settings.RootBeamtimesFolder + string(filepath.Separator)+test.second_folder
		}
		path_in_token:=abs_path
		if test.auto {
			path_in_token = "auto"
		}
		request :=  makeRequest(folderTokenRequest{path_in_token,test.beamtime_id,test.token, "instance", "step"})
		if test.status == http.StatusBadRequest {
			request =makeRequest(authorizationRequest{})
		} else {
			mock_store.On("IsTokenRevoked", mock.Anything).Return(false, nil)
		}
		w := doPostRequest("/"+version.GetAuthorizerApiVersion()+"/folder",request,"")
		if w.Code == http.StatusOK {
			body, _ := ioutil.ReadAll(w.Body)
			claims,_ := utils.CheckJWTToken(string(body),"secret_folder")
			var extra_claim structs.FolderTokenTokenExtraClaim
			utils.MapToStruct(claims.(*utils.CustomClaims).ExtraClaims.(map[string]interface{}), &extra_claim)
			assert.Equal(t, filepath.Clean(abs_path), filepath.Clean(extra_claim.RootFolder), testName)
			assert.Equal(t, filepath.Clean(abs_path_second), filepath.Clean(extra_claim.SecondFolder), testName)
		} else {
			body, _ := ioutil.ReadAll(w.Body)
			fmt.Println(string(body))
		}

		assert.Equal(t, test.status, w.Code, testName)
		mock_store.AssertExpectations(t)
		mock_store.ExpectedCalls = nil
		mock_store.Calls = nil
		assert.Equal(t, test.status, w.Code, test.message)
	}
}

func TestFolderTokenWrongProtocol(t *testing.T) {
		request :=  makeRequest(folderTokenRequest{"abs_path","beamtime_id","token", "instance", "step"})
		w := doPostRequest("/v10000.2/folder",request,"")
		assert.Equal(t, http.StatusUnsupportedMediaType, w.Code, "wrong protocol")
}
