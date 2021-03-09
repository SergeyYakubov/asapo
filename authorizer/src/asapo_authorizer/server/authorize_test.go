package server

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/common"
	"asapo_authorizer/ldap_client"
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


func prepareToken(payload string) string{
	Auth = authorization.NewAuth(utils.NewHMACAuth("secret"),nil,nil)
	token, _ := Auth.UserAuth().GenerateToken(&payload)
	return token
}

func prepareAdminToken(payload string) string{
	Auth = authorization.NewAuth(nil,utils.NewJWTAuth("secret_admin"),nil)

	var claims utils.CustomClaims
	var extraClaim utils.AccessTokenExtraClaim
	claims.Subject = payload
	extraClaim.AccessType = "create"
	claims.ExtraClaims = &extraClaim
	token, _ := Auth.AdminAuth().GenerateToken(&claims)
	return token
}

var mockClient = new(ldap_client.MockedLdapClient)


type request struct {
	path    string
	cmd     string
	answer  int
	message string
}

func allowBeamlines(beamlines []beamtimeMeta) {
	settings.AlwaysAllowedBeamtimes=beamlines
}


func containsMatcher(substr string) func(str string) bool {
	return func(str string) bool { return strings.Contains(str, substr) }
}

func makeRequest(request interface{}) string {
	buf, _ := utils.MapToJson(request)
	return string(buf)
}

func doPostRequest(path string,buf string,authHeader string) *httptest.ResponseRecorder {
	mux := utils.NewRouter(listRoutes)
	req, _ := http.NewRequest("POST", path, strings.NewReader(buf))
	if authHeader!="" {
		req.Header.Add("Authorization",authHeader)
	}
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
	{"processed%asapo_test%auto%%", SourceCredentials{"asapo_test","auto","detector","","processed"},true,"auto beamline, source and no token"},
	{"processed%asapo_test%auto%%token", SourceCredentials{"asapo_test","auto","detector","token","processed"},true,"auto beamline, source"},
	{"processed%asapo_test%auto%source%", SourceCredentials{"asapo_test","auto","source","","processed"},true,"auto beamline, no token"},
	{"processed%asapo_test%auto%source%token", SourceCredentials{"asapo_test","auto","source","token","processed"},true,"auto beamline,source, token"},
	{"processed%asapo_test%beamline%source%token", SourceCredentials{"asapo_test","beamline","source","token","processed"},true,"all set"},
	{"processed%auto%beamline%source%token", SourceCredentials{"auto","beamline","source","token","processed"},true,"auto beamtime"},
	{"raw%auto%auto%source%token", SourceCredentials{},false,"auto beamtime and beamline"},
	{"raw%%beamline%source%token", SourceCredentials{"auto","beamline","source","token","raw"},true,"empty beamtime"},
	{"raw%asapo_test%%source%token", SourceCredentials{"asapo_test","auto","source","token","raw"},true,"empty bealine"},
	{"raw%%%source%token", SourceCredentials{},false,"both empty"},
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
	allowBeamlines([]beamtimeMeta{{"asapo_test","beamline","","2019","tf",""}})
	request :=  makeRequest(authorizationRequest{"processed%asapo_test%%%","host"})
	w := doPostRequest("/authorize",request,"")

	body, _ := ioutil.ReadAll(w.Body)

	assert.Contains(t, string(body), "asapo_test", "")
	assert.Contains(t, string(body), "beamline", "")
	assert.Contains(t, string(body), "detector", "")
	assert.Contains(t, string(body), "processed", "")

	assert.Equal(t, http.StatusOK, w.Code, "")
}

var beamtime_meta_online =`
{
"beamline": "bl1",
"beamtimeId": "test_online"
}
`

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
"core-path": "asap3/petra3/gpfs/p07/2020/data/11111111",
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

var authTests = [] struct {
	source_type string
	beamtime_id string
	beamline string
	dataSource string
	token string
	originHost string
	status int
	message string
	answer string
}{
	{"processed","test","auto","dataSource", prepareToken("test"),"127.0.0.2",http.StatusOK,"user source with correct token",
		`{"beamtimeId":"test","beamline":"bl1","dataSource":"dataSource","core-path":"./tf/gpfs/bl1/2019/data/test","beamline-path":"","source-type":"processed"}`},
	{"processed","test_online","auto","dataSource", prepareToken("test_online"),"127.0.0.1",http.StatusOK,"with online path, processed type",
		`{"beamtimeId":"test_online","beamline":"bl1","dataSource":"dataSource","core-path":"./tf/gpfs/bl1/2019/data/test_online","beamline-path":"","source-type":"processed"}`},
	{"processed","test1","auto","dataSource", prepareToken("test1"),"127.0.0.1",http.StatusUnauthorized,"correct token, beamtime not found",
		""},
	{"processed","test","auto","dataSource", prepareToken("wrong"),"127.0.0.1",http.StatusUnauthorized,"user source with wrong token",
		""},
	{"processed","test","bl1","dataSource", prepareToken("test"),"127.0.0.1",http.StatusOK,"correct beamline given",
		`{"beamtimeId":"test","beamline":"bl1","dataSource":"dataSource","core-path":"./tf/gpfs/bl1/2019/data/test","beamline-path":"","source-type":"processed"}`},
		{"processed","test","bl2","dataSource", prepareToken("test"),"127.0.0.1",http.StatusUnauthorized,"incorrect beamline given",
		""},
	{"processed","auto","p07", "dataSource",prepareToken("bl_p07"),"127.0.0.1",http.StatusOK,"beamtime found",
		`{"beamtimeId":"11111111","beamline":"p07","dataSource":"dataSource","core-path":"asap3/petra3/gpfs/p07/2020/data/11111111","beamline-path":"","source-type":"processed"}`},
	{"processed","auto","p07", "dataSource",prepareToken("bl_p06"),"127.0.0.1",http.StatusUnauthorized,"wrong token",
		""},
	{"processed","auto","p08", "dataSource",prepareToken("bl_p08"),"127.0.0.1",http.StatusUnauthorized,"beamtime not found",
		""},
	{"raw","test_online","auto","dataSource", prepareToken("test_online"),"127.0.0.1",http.StatusOK,"raw type",
		`{"beamtimeId":"test_online","beamline":"bl1","dataSource":"dataSource","core-path":"./tf/gpfs/bl1/2019/data/test_online","beamline-path":"./bl1/current","source-type":"raw"}`},
	{"raw","test_online","auto","dataSource", "","127.0.0.1",http.StatusOK,"raw type",
		`{"beamtimeId":"test_online","beamline":"bl1","dataSource":"dataSource","core-path":"./tf/gpfs/bl1/2019/data/test_online","beamline-path":"./bl1/current","source-type":"raw"}`},
 	{"raw","auto","p07","dataSource", "","127.0.0.1",http.StatusOK,"raw type, auto beamtime",
		`{"beamtimeId":"11111111","beamline":"p07","dataSource":"dataSource","core-path":"asap3/petra3/gpfs/p07/2020/data/11111111","beamline-path":"./p07/current","source-type":"raw"}`},
	{"raw","auto","p07","noldap", "","127.0.0.1",http.StatusNotFound,"no conection to ldap",
		""},
	{"raw","test_online","auto","dataSource", "","127.0.0.2",http.StatusUnauthorized,"raw type, wrong origin host",
		""},
	{"raw","test","auto","dataSource", prepareToken("test"),"127.0.0.1",http.StatusUnauthorized,"raw when not online",
		""},
	{"processed","test","auto","dataSource", "","127.0.0.1:1001",http.StatusOK,"processed without token",
		`{"beamtimeId":"test","beamline":"bl1","dataSource":"dataSource","core-path":"./tf/gpfs/bl1/2019/data/test","beamline-path":"","source-type":"processed"}`},
	{"processed","test","auto","dataSource", "","127.0.0.2",http.StatusUnauthorized,"processed without token, wrong host",
		""},
}

func TestAuthorize(t *testing.T) {
	ldapClient = mockClient
	allowBeamlines([]beamtimeMeta{})
	Auth = authorization.NewAuth(utils.NewHMACAuth("secret"),utils.NewHMACAuth("secret"),utils.NewJWTAuth("secret"))
	expected_uri := "expected_uri"
	expected_base := "expected_base"
	allowed_ips := []string{"127.0.0.1"}
	settings.RootBeamtimesFolder ="."
	settings.CurrentBeamlinesFolder="."
	settings.Ldap.FilterTemplate="a3__BEAMLINE__-hosts"
	settings.Ldap.Uri = expected_uri
	settings.Ldap.BaseDn = expected_base

	os.MkdirAll(filepath.Clean("tf/gpfs/bl1/2019/data/test"), os.ModePerm)
	os.MkdirAll(filepath.Clean("tf/gpfs/bl1/2019/data/test_online"), os.ModePerm)
	os.MkdirAll(filepath.Clean("p07/current"), os.ModePerm)
	os.MkdirAll(filepath.Clean("bl1/current"), os.ModePerm)
	ioutil.WriteFile(filepath.Clean("p07/current/beamtime-metadata-11111111.json"), []byte(beamtime_meta), 0644)
	ioutil.WriteFile(filepath.Clean("bl1/current/beamtime-metadata-test_online.json"), []byte(beamtime_meta_online), 0644)
	defer 	os.RemoveAll("p07")
	defer 	os.RemoveAll("tf")
	defer 	os.RemoveAll("bl1")

	for _, test := range authTests {
		if test.source_type == "raw" || test.token == "" {bl := test.beamline
			if test.beamline == "auto" {
				bl = "bl1"
			}
			expected_filter:="a3"+bl+"-hosts"
			if test.dataSource == "noldap" {
				err := &common.ServerError{utils.StatusServiceUnavailable,""}
				mockClient.On("GetAllowedIpsForBeamline", expected_uri, expected_base,expected_filter).Return([]string{}, err)
			} else {
				mockClient.On("GetAllowedIpsForBeamline", expected_uri, expected_base,expected_filter).Return(allowed_ips, nil)
			}
		}

		request :=  makeRequest(authorizationRequest{test.source_type+"%"+test.beamtime_id+"%"+test.beamline+"%"+test.dataSource+"%"+test.token,test.originHost})
		w := doPostRequest("/authorize",request,"")

		body, _ := ioutil.ReadAll(w.Body)
		if test.status==http.StatusOK {
			body_str:=string(body)
			body_str = strings.Replace(body_str,string(os.PathSeparator),"/",-1)
			body_str = strings.Replace(body_str,"//","/",-1)
			assert.Equal(t, test.answer,body_str,test.message)
		}
		assert.Equal(t, test.status,w.Code, test.message)
		mockClient.AssertExpectations(t)
		mockClient.ExpectedCalls=nil
	}
}

func TestNotAuthorized(t *testing.T) {
	request :=  makeRequest(authorizationRequest{"raw%any_id%%%","host"})
	w := doPostRequest("/authorize",request,"")
	assert.Equal(t, http.StatusUnauthorized, w.Code, "")
}


func TestAuthorizeWrongRequest(t *testing.T) {
	w := doPostRequest("/authorize","babla","")
	assert.Equal(t, http.StatusBadRequest, w.Code, "")
}


func TestAuthorizeWrongPath(t *testing.T) {
	w := doPostRequest("/authorized","","")
	assert.Equal(t, http.StatusNotFound, w.Code, "")
}

func TestDoNotAuthorizeIfNotInAllowed(t *testing.T) {
	allowBeamlines([]beamtimeMeta{{"test","beamline","","2019","tf",""}})

	request :=  authorizationRequest{"asapo_test%%","host"}
	creds,_ := getSourceCredentials(request)
	_,err := authorize(request,creds)
	assert.Error(t,err, "")
}

func TestSplitHost(t *testing.T) {
	host := splitHost("127.0.0.1:112")
	assert.Equal(t,"127.0.0.1", host, "")
}


func TestSplitHostNoPort(t *testing.T) {
	host := splitHost("127.0.0.1")
	assert.Equal(t,"127.0.0.1", host, "")
}

var extractBtinfoTests = [] struct {
	root string
	fname string
	beamline string
	id string
	ok bool
}{
	{".",filepath.Clean("tf/gpfs/bl1.01/2019/data/123"),"bl1.01","123",true},
	{filepath.Clean("/blabla/tratartra"),filepath.Clean("tf/gpfs/bl1.01/2019/data/123"), "bl1.01","123",true},
	{".",filepath.Clean("tf/gpfs/common/2019/data/123"), "bl1.01","123",false},
	{".",filepath.Clean("tf/gpfs/BeamtimeUsers/2019/data/123"), "bl1.01","123",false},
	{".",filepath.Clean("tf/gpfs/state/2019/data/123"), "bl1.01","123",false},
	{".",filepath.Clean("tf/gpfs/support/2019/data/123"), "bl1.01","123",false},
	{".",filepath.Clean("petra3/gpfs/p01/2019/comissioning/c20180508-000-COM20181"), "p01","c20180508-000-COM20181",true},

}
func TestGetBeamtimeInfo(t *testing.T) {
	for _, test := range extractBtinfoTests {
		settings.RootBeamtimesFolder=test.root
		bt,err:= beamtimeMetaFromMatch(test.root+string(filepath.Separator)+test.fname)
		if test.ok {
			assert.Equal(t,bt.OfflinePath,test.root+string(filepath.Separator)+test.fname)
			assert.Equal(t,bt.Beamline,test.beamline)
			assert.Equal(t,bt.BeamtimeId,test.id)
			assert.Nil(t,err,"should not be error")
		} else {
			assert.NotNil(t,err,"should be error")
		}
	}

}
