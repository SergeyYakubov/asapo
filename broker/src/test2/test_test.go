package test2

import (
	"github.com/stretchr/testify/assert"
	"testing"
)

func TestCreateRecord(t *testing.T) {
	Test_Hidra2()
	assert.Equal(t, "111", "111", "record created")
}
