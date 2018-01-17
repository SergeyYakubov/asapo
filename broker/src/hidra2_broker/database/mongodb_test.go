// +build integration_tests

package database

import (
	"github.com/stretchr/testify/assert"
	"testing"
)

// these are tjhe integration tests. They assume mongo db is runnig on 127.0.0.1:27027
// test names shlud contain MongoDB*** so that go test could find them:
// go_integration_test(${TARGET_NAME}-connectdb "./..." "MongoDBConnect")
func TestMongoDBConnectFails(t *testing.T) {
	var db Mongodb
	err := db.Connect("blabla")
	assert.NotNil(t, err)
}

func TestMongoDBConnectOK(t *testing.T) {
	var db Mongodb
	err := db.Connect("127.0.0.1:27017")
	assert.Nil(t, err)
	db.Close()
}
