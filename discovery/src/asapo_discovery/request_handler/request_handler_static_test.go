package request_handler

import (
	"github.com/stretchr/testify/assert"
	"testing"
)


var uris = []string{"ip1","ip2"}
const max_conn = 1

var rh StaticRequestHandler;

func TestStaticHandlerInitOK(t *testing.T) {
	err := rh.Init(max_conn,uris)
	assert.Nil(t, err)
}

func TestStaticHandlerGetOK(t *testing.T) {
	rh.Init(max_conn,uris)
	res,err := rh.GetReceivers()
	assert.Equal(t,string(res), "{\"MaxConnections\":1,\"Uris\":[\"ip1\",\"ip2\"]}")
	assert.Nil(t, err)

}
