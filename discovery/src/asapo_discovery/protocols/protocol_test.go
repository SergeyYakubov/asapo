package protocols

import (
	"github.com/stretchr/testify/assert"
	"testing"
)

type protocolTest struct {
	client   string
	protocol string
	result   bool
	hint     string
	message  string
}

var protocolTests = []protocolTest{
// consumer
	{"consumer", "v0.4", true, "current", "v0.4"},
	{"consumer", "v0.3", true, "deprecates", "v0.3"},
	{"consumer", "v0.2", true, "deprecates", "v0.2"},
	{"consumer", "v0.1", true, "deprecates", "v0.1"},
	{"consumer", "v1000.2", false, "unknown", "unknown protocol"},



// producer
	{"producer", "v0.4", true, "current", "v0.4"},
	{"producer", "v0.3", true, "deprecates", "v0.3"},
	{"producer", "v0.2", true, "deprecates", "v0.2"},
	{"producer", "v0.1", true, "deprecates", "v0.1"},
	{"producer", "v1000.2", false, "unknown", "unknown protocol"},
}

func TestProtocolTests(t *testing.T) {
	for _, ct := range protocolTests {
		hint, ok := ValidateProtocol(ct.client, ct.protocol)
		assert.Equal(t, ct.result, ok, ct.message)
		assert.Contains(t, hint, ct.hint, ct.message)
	}
}
