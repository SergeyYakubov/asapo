package server

import (
	"asapo_common/version"
	"asapo_discovery/protocols"
	"encoding/json"
	"fmt"
	"github.com/stretchr/testify/assert"
	"net/http"
	"testing"
)

var coreVer = version.GetVersion()

var versionTests = []struct {
	request string
	result  versionInfo
	code    int
	message string
}{
	{"", versionInfo{
		SoftwareVersion:        coreVer,
		ClientProtocol: protocols.ProtocolInfo{},
		ClientSupported:        "",
	}, http.StatusOK, "no client"},
	{"?client=consumer", versionInfo{
		SoftwareVersion: coreVer,
		ClientProtocol:     protocols.ProtocolInfo{"", nil},
		ClientSupported:            "no",
	}, http.StatusOK, "consumer client, no protocol"},

	{"?client=consumer&protocol=v0.1", versionInfo{
		SoftwareVersion: coreVer,
		ClientProtocol:     protocols.ProtocolInfo{"v0.1 (current)",
			map[string]string{"Authorizer":"v0.1", "Broker":"v0.1", "Data cache service":"v0.1", "Discovery":"v0.1", "File Transfer":"v0.1"}},
		ClientSupported:            "yes",
	}, http.StatusOK, "consumer client"},
	{"?client=producer&protocol=v0.1", versionInfo{
		SoftwareVersion:        coreVer,
		ClientProtocol: protocols.ProtocolInfo{"v0.1 (current)",map[string]string{"Discovery":"v0.1", "Receiver":"v0.1"}},
		ClientSupported:        "yes",
	}, http.StatusOK, "producer client"},
	{"?client=producer&protocol=v0.2", versionInfo{
		SoftwareVersion:        coreVer,
		ClientProtocol: protocols.ProtocolInfo{"v0.2 (unknown protocol)",nil},
		ClientSupported:        "no",
	}, http.StatusOK, "producer client unknown"},
}

func TestVersionTests(t *testing.T) {
	for _, test := range versionTests {
		w := doRequest("/" + version.GetDiscoveryApiVersion() + "/version" + test.request)
		assert.Equal(t, test.code, w.Code, test.message)
		if test.code == http.StatusOK {
			var info versionInfo
			json.Unmarshal(w.Body.Bytes(), &info)
			fmt.Println(w.Body.String())
			assert.Equal(t, test.result.ClientProtocol,info.ClientProtocol, test.message)
			if test.message!="no client" {
				assert.Equal(t, true,len(info.SupportedProtocols)>0, test.message)
			}
		}
	}
}

func TestVersionTestsWrongApi(t *testing.T) {
	w := doRequest("/v2.0/version")
	assert.Equal(t, http.StatusUnsupportedMediaType, w.Code, "wrong api")
}
