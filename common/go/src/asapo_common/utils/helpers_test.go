package utils

import (
	"github.com/stretchr/testify/assert"
	"testing"
)



var encodeTests = []struct {
	input       string
	str1        string
	str2       string
	message    string
}{
	{"5/hellohello1","hello", "hello1","ok"},
	{"0/","", "","ok"},
	{"5/hello","hello", "","ok"},
	{"10/hello_testall","hello_test", "all","ok"},

}

func TestEncodeDecode(t *testing.T) {
	for _, test := range encodeTests {
		encoded:=EncodeTwoStrings(test.str1,test.str2)
		assert.Equal(t, test.input,encoded,test.message)
		s1,s2,err:=DecodeTwoStrings(test.input)
		assert.Equal(t, test.str1,s1,test.message)
		assert.Equal(t, test.str2,s2,test.message)
		assert.NoError(t, err,test.message)
	}
}

