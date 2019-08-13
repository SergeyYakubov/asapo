package server

import (
	"asapo_common/utils"
	"github.com/stretchr/testify/assert"
	"io/ioutil"
	"net/http"
	"net/http/httptest"
	"os"
	"strings"
	"testing"
)

func prepareToken(beamtime_id string) string{
	auth = utils.NewHMACAuth("secret")
	token, _ := auth.GenerateToken(&beamtime_id)
	return token
}


type request struct {
	path    string
	cmd     string
	answer  int
	message string
}

func allowBeamlines(beamlines []beamtimeInfo) {
	settings.AlwaysAllowedBeamtimes=beamlines
}


func containsMatcher(substr string) func(str string) bool {
	return func(str string) bool { return strings.Contains(str, substr) }
}

func makeRequest(request authorizationRequest) string {
	buf, _ := utils.MapToJson(request)
	return string(buf)
}

func doAuthorizeRequest(path string,buf string) *httptest.ResponseRecorder {
	mux := utils.NewRouter(listRoutes)
	req, _ := http.NewRequest("POST", path, strings.NewReader(buf))
	w := httptest.NewRecorder()
	mux.ServeHTTP(w, req)
	return w
}


var credTests = [] struct {
	request string
	cred SourceCredentials
	message string
} {
	{"asapo_test%%", SourceCredentials{"asapo_test","detector",""},"default stream and no token"},
	{"asapo_test%%token", SourceCredentials{"asapo_test","detector","token"},"default stream"},
	{"asapo_test%stream%", SourceCredentials{"asapo_test","stream",""},"no token"},
	{"asapo_test%stream%token", SourceCredentials{"asapo_test","stream","token"},"all set"},
}

func TestSplitCreds(t *testing.T) {

	for _, test := range credTests {
		request :=  authorizationRequest{test.request,"host"}
		creds,err := getSourceCredentials(request)
		assert.Nil(t,err)
		assert.Equal(t,creds,test.cred,test.message)
	}
}

func TestAuthorizeDefaultOK(t *testing.T) {
	allowBeamlines([]beamtimeInfo{{"asapo_test","beamline",""}})
	request :=  makeRequest(authorizationRequest{"asapo_test%%","host"})
	w := doAuthorizeRequest("/authorize",request)

	body, _ := ioutil.ReadAll(w.Body)

	assert.Contains(t, string(body), "asapo_test", "")
	assert.Contains(t, string(body), "beamline", "")
	assert.Contains(t, string(body), "detector", "")

	assert.Equal(t, http.StatusOK, w.Code, "")
}

var authTests = [] struct {
	stream string
	token string
	status int
	message string
}{
	{"stream", prepareToken("test"),http.StatusOK,"user stream with correct token"},
	{"stream", prepareToken("wrong"),http.StatusUnauthorized,"user stream with wrong token"},
	{"detector_aaa", prepareToken("test"),http.StatusUnauthorized,"detector stream with correct token and wroung source"},
}
func TestAuthorizeWithToken(t *testing.T) {
	allowBeamlines([]beamtimeInfo{})
	settings.BeamtimeBeamlineMappingFile="file.tmp"
	beamtime_id:= "test"
	lines :="line1\n line2\n flash bl1 "+ beamtime_id	+" start: 2018-04-24"
	ioutil.WriteFile("file.tmp", []byte(lines), 0644)
	defer 	os.Remove("file.tmp")

	for _, test := range authTests {
		request :=  makeRequest(authorizationRequest{beamtime_id+"%"+test.stream+"%"+test.token,"host"})
		w := doAuthorizeRequest("/authorize",request)

		body, _ := ioutil.ReadAll(w.Body)
		if test.status==http.StatusOK {
			assert.Contains(t, string(body), beamtime_id, "")
			assert.Contains(t, string(body), "bl1", "")
			assert.Contains(t, string(body), test.stream, "")
		}

		assert.Equal(t, test.status, w.Code, test.message)
	}


}



func TestNotAuthorized(t *testing.T) {
	request :=  makeRequest(authorizationRequest{"any_id%%","host"})
	w := doAuthorizeRequest("/authorize",request)
	assert.Equal(t, http.StatusUnauthorized, w.Code, "")
}


func TestAuthorizeWrongRequest(t *testing.T) {
	w := doAuthorizeRequest("/authorize","babla")
	assert.Equal(t, http.StatusBadRequest, w.Code, "")
}


func TestAuthorizeWrongPath(t *testing.T) {
	w := doAuthorizeRequest("/authorized","")
	assert.Equal(t, http.StatusNotFound, w.Code, "")
}

func TestDoNotAuthorizeIfNotInAllowed(t *testing.T) {
	allowBeamlines([]beamtimeInfo{{"test","beamline",""}})
	request :=  authorizationRequest{"asapo_test%%","host"}
	creds,_ := getSourceCredentials(request)
	_,ok := authorize(request,creds)
	assert.Equal(t,false, ok, "")
}

func TestSplitHost(t *testing.T) {
	host := splitHost("127.0.0.1:112")
	assert.Equal(t,"127.0.0.1", host, "")
}


func TestSplitHostNoPort(t *testing.T) {
	host := splitHost("127.0.0.1")
	assert.Equal(t,"127.0.0.1", host, "")
}

func TestGetBeamlineFromIP(t *testing.T) {
	beamline, err := getBeamlineFromIP("127.0.0.1:112")
	assert.NotNil(t,err, "")
	assert.Empty(t,beamline, "")

}
func TestCheckBeamtimeExistsInStringsFalse(t *testing.T) {
	beamInfo := beamtimeInfo{"123","bl",""}
	lines:=[]string{"111","flash	pg2	11003932		beamtime	start: 2018-06-11","petra3	p01	c20180508-000-COM20181	commissioning"}
	bl,ok := checkBeamtimeExistsInStrings(beamInfo.BeamtimeId,lines)
	assert.False(t,ok, "")
	assert.Equal(t,"",bl, "")
}


func TestCheckBeamtimeExistsInStringsOk(t *testing.T) {
	beamInfo := beamtimeInfo{"11003932","pg2",""}
	lines:=[]string{"111","flash	pg2	11003932		beamtime	start: 2018-06-11","petra3	p01	c20180508-000-COM20181	commissioning"}
	bl,ok := checkBeamtimeExistsInStrings(beamInfo.BeamtimeId,lines)
	assert.True(t,ok, "")
	assert.Equal(t,bl,beamInfo.Beamline,"")

}

func TestAuthorizeWithFile(t *testing.T) {
	settings.IpBeamlineMappingFolder="."
	settings.BeamtimeBeamlineMappingFile="file.tmp"

	lines :=`
Open beam times as of  Thursday, 2018/06/21 11:32
Faclty	BL	BeamTime Id		kind
flash	bl1	11003924		beamtime	start: 2018-04-24
flash	bl2	11003921		beamtime	start: 2018-06-08
flash	fl24	11001734		beamtime	start: 2018-06-13
flash	pg2	11003932		beamtime	start: 2018-06-11
flash	thz	11005667		beamtime	start: 2018-05-24
petra3	ext	50000181		beamtime	start: 2017-04-12
petra3	ext	50000193		beamtime	start: 2017-10-12
petra3	ext	50000202		beamtime	start: 2017-12-06
petra3	ext	50000209		beamtime	start: 2018-02-19
petra3	ext	50000211		beamtime	start: 2018-02-19
petra3	ext	50000214		beamtime	start: 2018-04-23
petra3	ext	50000215		beamtime	start: 2018-03-23
petra3	ext	50000216		beamtime	start: 2018-03-23
petra3	ext	50000217		beamtime	start: 2018-03-23
petra3	ext	50000218		beamtime	start: 2018-03-23
petra3	ext	50000219		beamtime	start: 2018-04-24
petra3	ext	50000221		beamtime	start: 2018-06-14
petra3	p01	11004172		beamtime	start: 2018-06-20
petra3	p01	c20180508-000-COM20181	commissioning
petra3	p02.1	11004341		beamtime	start: 2018-06-18
`

	ioutil.WriteFile("file.tmp", []byte(lines), 0644)
	ioutil.WriteFile("127.0.0.1", []byte("bl1"), 0644)


	request := authorizationRequest{"11003924%%","127.0.0.1"}
	w := doAuthorizeRequest("/authorize",makeRequest(request))

	body, _ := ioutil.ReadAll(w.Body)
	assert.Contains(t, string(body), "11003924", "")
	assert.Contains(t, string(body), "bl1", "")
	assert.Equal(t, http.StatusOK, w.Code, "")

	request = authorizationRequest{"wrong%%","127.0.0.1"}
	w = doAuthorizeRequest("/authorize",makeRequest(request))
	assert.Equal(t, http.StatusUnauthorized, w.Code, "")

	os.Remove("127.0.0.1")
	os.Remove("file.tmp")

}


