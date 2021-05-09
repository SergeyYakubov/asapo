package server

import (
	"asapo_common/logger"
	"github.com/rs/xid"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"net/http"
	"testing"
)

func GetObjectID(t *testing.T) (xid.ID, error) {
	w := doRequest("/creategroup", "POST")
	assert.Equal(t, http.StatusOK, w.Code, "New Group OK")
	return xid.FromString(w.Body.String())
}

func TestGetNewGroup(t *testing.T) {
	statistics.Reset()
	logger.SetMockLog()
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("generated new group")))

	id1, err := GetObjectID(t)
	assert.Nil(t, err, "first is ObjectID")

	id2, err := GetObjectID(t)
	assert.Nil(t, err, "second is ObjectID")

	assert.NotEqual(t, id1.String(), id2.String())
	assert.Equal(t, id1.Counter()+1, id2.Counter())
	assert.Equal(t, 2, statistics.GetCounter(), "creategroup increases counter")

	logger.UnsetMockLog()
}

func TestGetNewGroupWrongProtocol(t *testing.T) {
	w := doRequest("/creategroup", "POST","","/v1.2")
	assert.Equal(t, http.StatusUnsupportedMediaType, w.Code, "wrong request")
}
