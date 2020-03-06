package server

import (
	"asapo_common/utils"
	"github.com/stretchr/testify/assert"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
	"io/ioutil"
	"time"
	"os"
	"path/filepath"
)


type request struct {
	path    string
	cmd     string
	answer  int
	message string
}

func containsMatcher(substr string) func(str string) bool {
	return func(str string) bool { return strings.Contains(str, substr) }
}

func makeRequest(request interface{}) string {
	buf, _ := utils.MapToJson(request)
	return string(buf)
}

func prepareToken(folder string) string{
	auth := utils.NewJWTAuth("key")

	var claims utils.CustomClaims
	var extraClaim utils.FolderTokenTokenExtraClaim
	extraClaim.RootFolder = folder
	claims.ExtraClaims = &extraClaim
	claims.Duration = time.Duration(1) * time.Minute
	token,_ := auth.GenerateToken(&claims)
	return token
}

func doPostRequest(path string,buf string,token string) *httptest.ResponseRecorder {
	mux := utils.NewRouter(listRoutes)
	req, _ := http.NewRequest("POST", path, strings.NewReader(buf))
	req.Header.Add("Authorization", token)
	w := httptest.NewRecorder()
	utils.ProcessJWTAuth(mux.ServeHTTP, "key")(w,req)
	return w
}

var transferFileTests = [] struct {
	folder string
	fname string
	token string
	status int
	message string
}{
	{"folder","exists", prepareToken("folder"),http.StatusOK,"file transferred"},
	{"folder","not_exists", prepareToken("folder"),http.StatusBadRequest,"file not exists"},
	{"wrong_folder","p07", prepareToken("folder"),http.StatusUnauthorized,"wrong folder"},
	{"folder","p07", "wrong token",http.StatusUnauthorized,"wrong token"},
}

func TestTransferFile(t *testing.T) {
	os.MkdirAll(filepath.Clean("folder"), os.ModePerm)
	ioutil.WriteFile(filepath.Clean("folder/exists"), []byte("hello"), 0644)
	defer 	os.RemoveAll("folder")

	for _, test := range transferFileTests {
		request :=  makeRequest(fileTransferRequest{test.folder,test.fname})
		w := doPostRequest("/transfer",request,test.token)
		if test.status==http.StatusOK {
			body, _ := ioutil.ReadAll(w.Body)
			body_str:=string(body)
			assert.Equal(t, test.status, w.Code, test.message)
			assert.Contains(t, w.Header().Get("Content-Disposition"),test.fname, test.message)
			assert.Equal(t, "hello", body_str, test.message)

		}
		assert.Equal(t, test.status, w.Code, test.message)
	}
}


