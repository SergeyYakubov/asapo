package server

import (
	"asapo_common/utils"
	"github.com/stretchr/testify/assert"
	"io/ioutil"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
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
	ok bool
	message string
} {
	{"asapo_test%auto%%", SourceCredentials{"asapo_test","auto","detector",""},true,"auto beamline, stream and no token"},
	{"asapo_test%auto%%token", SourceCredentials{"asapo_test","auto","detector","token"},true,"auto beamline, stream"},
	{"asapo_test%auto%stream%", SourceCredentials{"asapo_test","auto","stream",""},true,"auto beamline, no token"},
	{"asapo_test%auto%stream%token", SourceCredentials{"asapo_test","auto","stream","token"},true,"auto beamline,stream, token"},
	{"asapo_test%beamline%stream%token", SourceCredentials{"asapo_test","beamline","stream","token"},true,"all set"},
	{"auto%beamline%stream%token", SourceCredentials{"auto","beamline","stream","token"},true,"auto beamtime"},
	{"auto%auto%stream%token", SourceCredentials{},false,"auto beamtime and beamline"},
	{"%beamline%stream%token", SourceCredentials{"auto","beamline","stream","token"},true,"empty beamtime"},
	{"asapo_test%%stream%token", SourceCredentials{"asapo_test","auto","stream","token"},true,"empty bealine"},
	{"%%stream%token", SourceCredentials{},false,"both empty"},
}

func TestSplitCreds(t *testing.T) {

	for _, test := range credTests {
		request :=  authorizationRequest{test.request,"host"}
		creds,err := getSourceCredentials(request)
		if test.ok {
			assert.Nil(t,err)
			assert.Equal(t,creds,test.cred,test.message)
		} else {
			assert.NotNil(t,err,test.message)
		}

	}
}

func TestAuthorizeDefaultOK(t *testing.T) {
	allowBeamlines([]beamtimeInfo{{"asapo_test","beamline","","2019","tf"}})
	request :=  makeRequest(authorizationRequest{"asapo_test%%%","host"})
	w := doAuthorizeRequest("/authorize",request)

	body, _ := ioutil.ReadAll(w.Body)

	assert.Contains(t, string(body), "asapo_test", "")
	assert.Contains(t, string(body), "beamline", "")
	assert.Contains(t, string(body), "detector", "")

	assert.Equal(t, http.StatusOK, w.Code, "")
}

var authTests = [] struct {
	beamtime_id string
	beamline string
	stream string
	token string
	status int
	message string
}{
	{"test","auto","stream", prepareToken("test"),http.StatusOK,"user stream with correct token"},
	{"test1","auto","stream", prepareToken("test1"),http.StatusUnauthorized,"correct token, beamtime not found"},
	{"test","auto","stream", prepareToken("wrong"),http.StatusUnauthorized,"user stream with wrong token"},
	{"test","auto","detector_aaa", prepareToken("test"),http.StatusUnauthorized,"detector stream with correct token and wroung source"},
	{"test","bl1","stream", prepareToken("test"),http.StatusOK,"correct beamline given"},
	{"test","bl2","stream", prepareToken("test"),http.StatusUnauthorized,"incorrect beamline given"},
}
func TestAuthorizeWithToken(t *testing.T) {
	allowBeamlines([]beamtimeInfo{})
	settings.RootBeamtimesFolder ="."
	settings.CurrentBeamlinesFolder="."
	os.MkdirAll(filepath.Clean("tf/gpfs/bl1/2019/data/test"), os.ModePerm)
	os.MkdirAll(filepath.Clean("p07/current"), os.ModePerm)
	ioutil.WriteFile("p07/current/beamtime-metadata-11111111.json", []byte(beamtime_meta), 0644)

	defer 	os.RemoveAll("tf")
	defer 	os.RemoveAll("p07")

	for _, test := range authTests {
		request :=  makeRequest(authorizationRequest{test.beamtime_id+"%"+test.beamline+"%"+test.stream+"%"+test.token,"host"})
		w := doAuthorizeRequest("/authorize",request)

		body, _ := ioutil.ReadAll(w.Body)
		if test.status==http.StatusOK {
			assert.Contains(t, string(body), test.beamtime_id, "")
			assert.Contains(t, string(body), "bl1", "")
			assert.Contains(t, string(body), "stream", "")
			assert.Contains(t, string(body), "2019", "")
			assert.Contains(t, string(body), "tf", "")
			assert.Contains(t, string(body), test.stream, "")
		}

		assert.Equal(t, test.status, w.Code, test.message)
	}


}


var beamtime_meta =`
{
"applicant": {
"email": "test",
"institute": "test",
"lastname": "test",
"userId": "1234",
"username": "test"
},
"beamline": "p07",
"beamline_alias": "P07",
"beamtimeId": "11111111",
"contact": "None",
"core-path": "/asap3/petra3/gpfs/p07/2020/data/11111111",
"event-end": "2020-03-03 09:00:00",
"event-start": "2020-03-02 09:00:00",
"facility": "PETRA III",
"generated": "2020-02-22 22:37:16",
"pi": {
"email": "test",
"institute": "test",
"lastname": "test",
"userId": "14",
"username": "test"
},
"proposalId": "12345678",
"proposalType": "H",
"title": "In-House Research (P07)",
"unixId": "None",
"users": {
"door-db": [
"test"
],
"special": [],
"unknown": []
}
}
`

var authBeamlineTests = [] struct {
	beamtime_id string
	beamline string
	token string
	status int
	message string
}{
	{"11111111","p07", prepareToken("1111111"),http.StatusOK,"beamtime found"},
	{"11111111","p08", prepareToken("1111111"),http.StatusUnauthorized,"beamtime not found"},
}
/*
func TestAuthorizeBeamline(t *testing.T) {
	allowBeamlines([]beamtimeInfo{})
	settings.CurrentBeamlinesFolder="."
	os.MkdirAll(filepath.Clean("p07/current"), os.ModePerm)
	ioutil.WriteFile("p07/current/beamtime-metadata-11111111.json", []byte(beamtime_meta), 0644)
	defer 	os.RemoveAll("p07")

	for _, test := range authTests {
		request :=  makeRequest(authorizationRequest{"auto%"+test.beamline+"%%"+test.token,"host"})
		w := doAuthorizeRequest("/authorize",request)

		body, _ := ioutil.ReadAll(w.Body)
		if test.status==http.StatusOK {
			assert.Contains(t, string(body), test.beamtime_id, "")
			assert.Contains(t, string(body), test.beamline, "")
//			assert.Contains(t, string(body), "2019", "")
			assert.Contains(t, string(body), "default", "")
		}

		assert.Equal(t, test.status, w.Code, test.message)
	}


}

*/

func TestNotAuthorized(t *testing.T) {
	request :=  makeRequest(authorizationRequest{"any_id%%%","host"})
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
	allowBeamlines([]beamtimeInfo{{"test","beamline","","2019","tf"}})

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

func TestAuthorizeWithFile(t *testing.T) {
	settings.IpBeamlineMappingFolder="."
	settings.RootBeamtimesFolder ="."
	os.MkdirAll(filepath.Clean("tf/gpfs/bl1/2019/data/11003924"), os.ModePerm)


	ioutil.WriteFile("127.0.0.1", []byte("bl1"), 0644)


	request := authorizationRequest{"11003924%%%","127.0.0.1"}
	w := doAuthorizeRequest("/authorize",makeRequest(request))

	body, _ := ioutil.ReadAll(w.Body)
	assert.Contains(t, string(body), "11003924", "")
	assert.Contains(t, string(body), "bl1", "")
	assert.Contains(t, string(body), "detector", "")
	assert.Contains(t, string(body), "2019", "")
	assert.Contains(t, string(body), "tf", "")
	assert.Equal(t, http.StatusOK, w.Code, "")

	request = authorizationRequest{"wrong%%%","127.0.0.1"}
	w = doAuthorizeRequest("/authorize",makeRequest(request))
	assert.Equal(t, http.StatusUnauthorized, w.Code, "")

	os.Remove("127.0.0.1")
	os.RemoveAll("tf")

}


var extractBtinfoTests = [] struct {
	root string
	fname string
	facility string
	beamline string
	year string
	id string
	ok bool
}{
	{".",filepath.Clean("tf/gpfs/bl1.01/2019/data/123"),"tf", "bl1.01","2019","123",true},
	{filepath.Clean("/blabla/tratartra"),filepath.Clean("tf/gpfs/bl1.01/2019/data/123"),"tf", "bl1.01","2019","123",true},
	{".",filepath.Clean("tf/gpfs/common/2019/data/123"),"tf", "bl1.01","2019","123",false},
	{".",filepath.Clean("tf/gpfs/BeamtimeUsers/2019/data/123"),"tf", "bl1.01","2019","123",false},
	{".",filepath.Clean("tf/gpfs/state/2019/data/123"),"tf", "bl1.01","2019","123",false},
	{".",filepath.Clean("tf/gpfs/support/2019/data/123"),"tf", "bl1.01","2019","123",false},
	{".",filepath.Clean("petra3/gpfs/p01/2019/comissioning/c20180508-000-COM20181"),"petra3", "p01","2019","c20180508-000-COM20181",true},

}
func TestGetBeamtimeInfo(t *testing.T) {
	for _, test := range extractBtinfoTests {
		settings.RootBeamtimesFolder=test.root
		bt,err:=beamtimeInfoFromMatch(test.root+string(filepath.Separator)+test.fname)
		if test.ok {
			assert.Equal(t,bt.Facility,test.facility)
			assert.Equal(t,bt.Beamline,test.beamline)
			assert.Equal(t,bt.Year,test.year)
			assert.Equal(t,bt.BeamtimeId,test.id)
			assert.Equal(t,bt.Facility,test.facility)
			assert.Nil(t,err,"should not be error")
		} else {
			assert.NotNil(t,err,"should be error")
		}
	}

}
