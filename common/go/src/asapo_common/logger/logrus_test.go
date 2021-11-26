package logger

import (
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
	l.WithFields(map[string]interface{}{"testmap1":1}).Info("aaa")
	assert.Contains(t, logStr(hook),"testmap1")

	hook.Reset()
	l.WithFields(map[string]interface{}{"testmap2":1}).Info("bbb")
	assert.NotContains(t, logStr(hook),"testmap1")

}
