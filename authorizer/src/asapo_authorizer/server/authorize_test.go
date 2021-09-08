package server

import (
	"asapo_authorizer/authorization"
	"asapo_authorizer/common"
	"asapo_authorizer/ldap_client"
	"asapo_common/structs"
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


func prepareUserToken(payload string, accessTypes []string) string{
	auth := authorization.NewAuth(nil,utils.NewJWTAuth("secret_user"),nil)
	var claims utils.CustomClaims
	var extraClaim structs.AccessTokenExtraClaim
	claims.Subject = payload
	extraClaim.AccessTypes = accessTypes
	claims.ExtraClaims = &extraClaim
	token, _ := auth.AdminAuth().GenerateToken(&claims)
	return token
}

func prepareAdminToken(payload string) string{
	auth:= authorization.NewAuth(nil,utils.NewJWTAuth("secret_admin"),nil)

	var claims utils.CustomClaims
	var extraClaim structs.AccessTokenExtraClaim
	claims.Subject = payload
	extraClaim.AccessTypes = []string{"create"}
	claims.ExtraClaims = &extraClaim
	token, _ := auth.AdminAuth().GenerateToken(&claims)
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
	newFormat bool
	request string
	cred SourceCredentials
	ok bool
	message string
} {
	{false, "processed%asapo_test%auto%%", SourceCredentials{"processed", "asapo_test","auto","detector","","Unset", "Unset"},true,"auto beamline, source and no token"},
	{false, "processed%asapo_test%auto%%token", SourceCredentials{"processed", "asapo_test","auto","detector","token","Unset", "Unset"},true,"auto beamline, source"},
	{false, "processed%asapo_test%auto%source%", SourceCredentials{"processed", "asapo_test","auto","source","","Unset", "Unset"},true,"auto beamline, no token"},
	{false, "processed%asapo_test%auto%source%token", SourceCredentials{"processed", "asapo_test","auto","source","token","Unset", "Unset"},true,"auto beamline,source, token"},
	{false, "processed%asapo_test%beamline%source%token", SourceCredentials{"processed", "asapo_test","beamline","source","token","Unset", "Unset"},true,"all set"},
	{false, "processed%auto%beamline%source%token", SourceCredentials{"processed", "auto","beamline","source","token","Unset", "Unset"},true,"auto beamtime"},
	{false, "raw%auto%auto%source%token", SourceCredentials{},false,"auto beamtime and beamline"},
	{false, "raw%%beamline%source%token", SourceCredentials{"raw", "auto","beamline","source","token", "Unset", "Unset"},true,"empty beamtime"},
	{false, "raw%asapo_test%%source%token", SourceCredentials{"raw", "asapo_test","auto","source","token","Unset", "Unset"},true,"empty bealine"},
	{false, "raw%%%source%token", SourceCredentials{},false,"both empty"},
	{false, "processed%asapo_test%beamline%source%blabla%token", SourceCredentials{"processed", "asapo_test","beamline","source%blabla","token","Unset", "Unset"},true,"% in source"},
	{false, "processed%asapo_test%beamline%source%blabla%", SourceCredentials{"processed", "asapo_test","beamline","source%blabla","","Unset", "Unset"},true,"% in source, no token"},
	{true, "processed%instance%step%asapo_test%beamline%source%blabla%", SourceCredentials{"processed", "asapo_test","beamline","source%blabla","","instance", "step"},true,"new format: % in source, no token"},
	{true, "processed%auto%step%asapo_test%beamline%source%blabla%", SourceCredentials{"processed", "asapo_test","beamline","source%blabla","","auto", "step"},false,"new format: auto instance"},
	{true, "processed%instance%auto%asapo_test%beamline%source%blabla%", SourceCredentials{"processed", "asapo_test","beamline","source%blabla","","instance", "auto"},false,"new format: auto step"},
	{true, "processed%%auto%asapo_test%beamline%source%blabla%", SourceCredentials{"processed", "asapo_test","beamline","source%blabla","","instance", "auto"},false,"new format: missing instance"},
	{true, "processed%instance%%asapo_test%beamline%source%blabla%", SourceCredentials{"processed", "asapo_test","beamline","source%blabla","","instance", "auto"},false,"new format: missing step"},
}

func TestSplitCreds(t *testing.T) {

	for _, test := range credTests {
		request :=  authorizationRequest{test.request,"host", test.newFormat}
		creds,err := getSourceCredentials(request)
		if test.ok {
			assert.Nil(t,err)
			assert.Equal(t,test.cred,creds,test.message)
		} else {
			assert.NotNil(t,err,test.message)
		}

	}
}

func TestAuthorizeDefaultOK(t *testing.T) {
	allowBeamlines([]beamtimeMeta{{"instance", "step", "asapo_test","beamline","","2019","tf","",nil}})
	request :=  makeRequest(authorizationRequest{"processed%asapo_test%%%","host", false})
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
"beamtimeId": "test_online",
"corePath": "asap3/petra3/gpfs/p07/2020/data/11111111"
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
"corePath": "asap3/petra3/gpfs/p07/2020/data/11111111",
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
	newFormat bool
	source_type string
	instance_id string
	pipeline_step string
	beamtime_id string
	beamline string
	dataSource string
	token string
	originHost string
	status int
	message string
	answer string
}{
	{true,"processed", "instance", "step", "test","auto","dataSource", prepareUserToken("bt_test",nil),"127.0.0.2",http.StatusUnauthorized,"missing access types",
		""},
	{true,"processed", "instance", "step", "test","auto","dataSource", prepareUserToken("bt_test",[]string{}),"127.0.0.2",http.StatusUnauthorized,"empty access types",
		""},
	{true,"processed", "instance", "step", "test","auto","dataSource", prepareUserToken("bt_test",[]string{"write"}),"127.0.0.2",http.StatusOK,"user source with correct token",
		`{"instanceId":"instance","pipelineStep":"step","beamtimeId":"test","beamline":"bl1","dataSource":"dataSource","corePath":"./tf/gpfs/bl1/2019/data/test","beamline-path":"","source-type":"processed","access-types":["write"]}`},
	{true,"processed", "instance", "step", "test_online","auto","dataSource", prepareUserToken("bt_test_online",[]string{"read"}),"127.0.0.1",http.StatusOK,"with online path, processed type",
		`{"instanceId":"instance","pipelineStep":"step","beamtimeId":"test_online","beamline":"bl1","dataSource":"dataSource","corePath":"./tf/gpfs/bl1/2019/data/test_online","beamline-path":"","source-type":"processed","access-types":["read"]}`},
	{true,"processed", "instance", "step", "test1","auto","dataSource", prepareUserToken("bt_test1",[]string{"read"}),"127.0.0.1",http.StatusUnauthorized,"correct token, beamtime not found",
		""},
	{true,"processed", "instance", "step", "test","auto","dataSource", prepareUserToken("wrong",[]string{"read"}),"127.0.0.1",http.StatusUnauthorized,"user source with wrong token",
		""},
	{true,"processed", "instance", "step", "test","bl1","dataSource", prepareUserToken("bt_test",[]string{"read"}),"127.0.0.1",http.StatusOK,"correct beamline given",
		`{"instanceId":"instance","pipelineStep":"step","beamtimeId":"test","beamline":"bl1","dataSource":"dataSource","corePath":"./tf/gpfs/bl1/2019/data/test","beamline-path":"","source-type":"processed","access-types":["read"]}`},
	{true,"processed", "instance", "step", "test","bl2","dataSource", prepareUserToken("bt_test",[]string{"read"}),"127.0.0.1",http.StatusUnauthorized,"incorrect beamline given",
		""},
	{true,"processed", "instance", "step", "auto","p07", "dataSource", prepareUserToken("bl_p07",[]string{"read"}),"127.0.0.1",http.StatusOK,"beamtime found",
		`{"instanceId":"instance","pipelineStep":"step","beamtimeId":"11111111","beamline":"p07","dataSource":"dataSource","corePath":"asap3/petra3/gpfs/p07/2020/data/11111111","beamline-path":"","source-type":"processed","access-types":["read"]}`},
	{true,"processed", "instance", "step", "auto","p07", "dataSource", prepareUserToken("bl_p06",[]string{"read"}),"127.0.0.1",http.StatusUnauthorized,"wrong token",
		""},
	{true,"processed", "instance", "step", "auto","p08", "dataSource", prepareUserToken("bl_p08",[]string{"read"}),"127.0.0.1",http.StatusUnauthorized,"beamtime not found",
		""},
	{true,"raw", "instance", "step", "test_online","auto","dataSource", "","127.0.0.1",http.StatusOK,"raw type",
		`{"instanceId":"instance","pipelineStep":"step","beamtimeId":"test_online","beamline":"bl1","dataSource":"dataSource","corePath":"./tf/gpfs/bl1/2019/data/test_online","beamline-path":"./bl1/current","source-type":"raw","access-types":["read","write"]}`},
	{true,"raw", "instance", "step", "test_online","auto","dataSource", "","127.0.0.1",http.StatusOK,"raw type",
		`{"instanceId":"instance","pipelineStep":"step","beamtimeId":"test_online","beamline":"bl1","dataSource":"dataSource","corePath":"./tf/gpfs/bl1/2019/data/test_online","beamline-path":"./bl1/current","source-type":"raw","access-types":["read","write"]}`},
 	{true,"raw", "instance", "step", "auto","p07","dataSource", "","127.0.0.1",http.StatusOK,"raw type, auto beamtime",
		`{"instanceId":"instance","pipelineStep":"step","beamtimeId":"11111111","beamline":"p07","dataSource":"dataSource","corePath":"asap3/petra3/gpfs/p07/2020/data/11111111","beamline-path":"./p07/current","source-type":"raw","access-types":["read","write"]}`},
	{true,"raw", "instance", "step", "auto","p07","noldap", "","127.0.0.1",http.StatusNotFound,"no conection to ldap",
		""},
	{true,"raw", "instance", "step", "test_online","auto","dataSource", "","127.0.0.2",http.StatusUnauthorized,"raw type, wrong origin host",
		""},
	{true,"raw", "instance", "step", "test","auto","dataSource", prepareUserToken("bt_test",[]string{"read"}),"127.0.0.1",http.StatusUnauthorized,"raw when not online",
		""},
	{true,"processed", "instance", "step", "test","auto","dataSource", "","127.0.0.1:1001",http.StatusOK,"processed without token",
		`{"instanceId":"instance","pipelineStep":"step","beamtimeId":"test","beamline":"bl1","dataSource":"dataSource","corePath":"./tf/gpfs/bl1/2019/data/test","beamline-path":"","source-type":"processed","access-types":["read","write"]}`},
	{true,"processed", "instance", "step", "test","auto","dataSource", "","127.0.0.2",http.StatusUnauthorized,"processed without token, wrong host",
		""},

	// Format testing
	{false,"processed", "", "", "test","bl1","dataSource", prepareUserToken("bt_test",[]string{"read"}),"127.0.0.1",http.StatusOK,"old format: correct beamline given",
		`{"instanceId":"Unset","pipelineStep":"Unset","beamtimeId":"test","beamline":"bl1","dataSource":"dataSource","corePath":"./tf/gpfs/bl1/2019/data/test","beamline-path":"","source-type":"processed","access-types":["read"]}`},
}

func TestAuthorize(t *testing.T) {
	ldapClient = mockClient
	allowBeamlines([]beamtimeMeta{})
	Auth = authorization.NewAuth(utils.NewJWTAuth("secret_user"),utils.NewJWTAuth("secret_admin"),utils.NewJWTAuth("secret"))
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
		if test.source_type == "raw" || test.token == "" {
			bl := test.beamline
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

		var sourceString string

		if test.newFormat {
			sourceString = test.source_type+"%"+test.instance_id+"%"+test.pipeline_step+"%"+test.beamtime_id+"%"+test.beamline+"%"+test.dataSource+"%"+test.token
		} else {
			sourceString = test.source_type+"%"+test.beamtime_id+"%"+test.beamline+"%"+test.dataSource+"%"+test.token
		}

		request := makeRequest(authorizationRequest{sourceString,test.originHost, test.newFormat})
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
	request :=  makeRequest(authorizationRequest{"raw%any_id%%%","host", false})
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
	allowBeamlines([]beamtimeMeta{{"", "", "test","beamline","","2019","tf","",nil}})

	request :=  authorizationRequest{"asapo_test%%","host", false}
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
