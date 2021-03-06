package database

import (
	"asapo_common/utils"
	"net/url"
)

const max_encoded_source_size = 63
const max_encoded_stream_size = 100
const max_encoded_group_size = 50

func shouldEscape(c byte, db bool) bool {
	if c == '$' || c == ' ' || c == '%' {
		return true
	}
	if !db {
		return false
	}

	switch c {
	case '\\', '/', '.', '"':
		return true
	}
	return false
}

const upperhex = "0123456789ABCDEF"

func escape(s string, db bool) string {
	hexCount := 0
	for i := 0; i < len(s); i++ {
		c := s[i]
		if shouldEscape(c, db) {
			hexCount++
		}
	}

	if hexCount == 0 {
		return s
	}

	var buf [64]byte
	var t []byte

	required := len(s) + 2*hexCount
	if required <= len(buf) {
		t = buf[:required]
	} else {
		t = make([]byte, required)
	}

	j := 0
	for i := 0; i < len(s); i++ {
		switch c := s[i]; {
		case shouldEscape(c, db):
			t[j] = '%'
			t[j+1] = upperhex[c>>4]
			t[j+2] = upperhex[c&15]
			j += 3
		default:
			t[j] = s[i]
			j++
		}
	}
	return string(t)
}

func encodeStringForDbName(original string) (result string) {
	return escape(original, true)
}


func decodeString(original string) (result string) {
	result,_ =  url.PathUnescape(original)
	return result
}

func encodeStringForColName(original string) (result string) {
	return escape(original, false)
}

func encodeRequest(request *Request) error {
	request.DataSource = encodeStringForDbName(request.DataSource)
	request.Beamtime = encodeStringForDbName(request.Beamtime)
	if len(request.DbName())> max_encoded_source_size {
		return &DBError{utils.StatusWrongInput, "source name is too long"}
	}

	request.Stream = encodeStringForColName(request.Stream)
	if len(request.Stream)> max_encoded_stream_size {
		return &DBError{utils.StatusWrongInput, "stream name is too long"}
	}

	request.GroupId = encodeStringForColName(request.GroupId)
	if len(request.GroupId)> max_encoded_group_size {
		return &DBError{utils.StatusWrongInput, "group id is too long"}
	}

	return nil
}
