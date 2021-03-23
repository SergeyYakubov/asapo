package server

import (
	"asapo_common/version"
	"asapo_discovery/common"
	"encoding/json"
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
		CoreServices:               coreVer,
		ClientConsumerProtocol:     "",
		ClientProducerProtocol:     "",
		ClientSupported:            "",
		SupportedProducerProtocols: []string{"v0.1 (current)"},
		SupportedConsumerProtocols: []string{"v0.1 (current)"},
	}, http.StatusOK, "no client"},
	{"?client=consumer&protocol=v0.1", versionInfo{
		CoreServices:               coreVer,
		ClientConsumerProtocol:     "v0.1 (current)",
		ClientProducerProtocol:     "",
		ClientSupported:            "yes",
		SupportedProducerProtocols: []string{"v0.1 (current)"},
		SupportedConsumerProtocols: []string{"v0.1 (current)"},
	}, http.StatusOK, "consumer client"},
	{"?client=producer&protocol=v0.1", versionInfo{
		CoreServices:               coreVer,
		ClientProducerProtocol:     "v0.1 (current)",
		ClientConsumerProtocol:     "",
		ClientSupported:            "yes",
		SupportedProducerProtocols: []string{"v0.1 (current)"},
		SupportedConsumerProtocols: []string{"v0.1 (current)"},
	}, http.StatusOK, "producer client"},
	{"?client=producer&protocol=v0.2", versionInfo{
		CoreServices:               coreVer,
		ClientProducerProtocol:     "v0.2 (unknown protocol)",
		ClientConsumerProtocol:     "",
		ClientSupported:            "no",
		SupportedProducerProtocols: []string{"v0.1 (current)"},
		SupportedConsumerProtocols: []string{"v0.1 (current)"},
	}, http.StatusOK, "producer client unknown"},
}

func TestVersionTests(t *testing.T) {
	for _, test := range versionTests {
		w := doRequest("/" + common.ApiVersion + "/version" + test.request)
		assert.Equal(t, test.code, w.Code, test.message)
		if test.code == http.StatusOK {
			var info versionInfo
			json.Unmarshal(w.Body.Bytes(), &info)
			assert.Equal(t, test.result, info, test.message)
		}
	}
}

func TestVersionTestsWrongApi(t *testing.T) {
	w := doRequest("/v2.0/version")
	assert.Equal(t, http.StatusUnsupportedMediaType, w.Code, "wrong api")
}
