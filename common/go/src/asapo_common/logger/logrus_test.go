package logger

import (
	"fmt"
	"github.com/sirupsen/logrus/hooks/test"
	"github.com/stretchr/testify/assert"
	"testing"
)

func logStr(hook *test.Hook) string {
	s := ""
	for _, entry := range hook.AllEntries() {
		ss, _ := entry.String()
		s += ss
	}
	return s
}

func TestLog(t *testing.T) {
	l := &logRusLogger{}
	hook := test.NewLocal(l.entry().Logger)
	l.WithFields(map[string]interface{}{"testmap":1}).Info("aaa")
	fmt.Println(logStr(hook))
	assert.Contains(t, logStr(hook),"testmap")

}
