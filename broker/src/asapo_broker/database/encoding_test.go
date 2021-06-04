package database

import (
	"github.com/stretchr/testify/assert"
	"testing"
)

func TestEncoding(t *testing.T) {
	stream:=`ss$`
	source :=`ads%&%41.sss`
	streamEncoded := encodeStringForColName(stream)
	sourceEncoded := encodeStringForDbName(source)
	streamDecoded := decodeString(streamEncoded)
	sourceDecoded := decodeString(sourceEncoded)
	assert.Equal(t, streamDecoded, stream)
	assert.Equal(t, sourceDecoded, source)

	r := Request{
		DbName:           source,
		DbCollectionName: stream,
		GroupId:          stream,
		Op:               "",
		DatasetOp:        false,
		MinDatasetSize:   0,
		ExtraParam:       "",
	}
	err := encodeRequest(&r)
	assert.Equal(t, r.DbCollectionName, streamEncoded)
	assert.Equal(t, r.GroupId, streamEncoded)
	assert.Equal(t, r.DbName, sourceEncoded)

	assert.Nil(t,err)
}
