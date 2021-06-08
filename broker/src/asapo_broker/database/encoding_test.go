package database

import (
	"asapo_common/utils"
	"github.com/stretchr/testify/assert"
	"math/rand"
	"testing"
)

func TestEncoding(t *testing.T) {
	stream := `ss$`
	source := `ads%&%41.sss`
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

	assert.Nil(t, err)
}

var encodeTests = []struct {
	streamSize int
	groupSize  int
	sourceSize int
	ok         bool
	message    string
}{
	{max_encoded_stream_size, max_encoded_group_size, max_encoded_source_size, true, "ok"},
	{max_encoded_stream_size + 1, max_encoded_group_size, max_encoded_source_size, false, "stream"},
	{max_encoded_stream_size, max_encoded_group_size + 1, max_encoded_source_size, false, "group"},
	{max_encoded_stream_size, max_encoded_group_size, max_encoded_source_size + 1, false, "source"},
}

func RandomString(n int) string {
	var letter = []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")

	b := make([]rune, n)
	for i := range b {
		b[i] = letter[rand.Intn(len(letter))]
	}
	return string(b)
}

func TestEncodingTooLong(t *testing.T) {
	for _, test := range encodeTests {
		stream := RandomString(test.streamSize)
		group := RandomString(test.groupSize)
		source := RandomString(test.sourceSize)
		r := Request{
			DbName:           source,
			DbCollectionName: stream,
			GroupId:          group,
			Op:               "",
			DatasetOp:        false,
			MinDatasetSize:   0,
			ExtraParam:       "",
		}
		err := encodeRequest(&r)
		if test.ok {
			assert.Nil(t, err, test.message)
		} else {
			assert.Equal(t, utils.StatusWrongInput, err.(*DBError).Code)
			assert.Contains(t,err.Error(),test.message,test.message)
		}
	}
}
