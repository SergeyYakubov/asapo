package utils

import (
	json "encoding/json"
	"io/ioutil"
)

func StringInSlice(a string, list []string) bool {
	for _, b := range list {
		if b == a {
			return true
		}
	}
	return false
}

func MapToJson(res interface{}) ([]byte, error) {
	answer, err := json.Marshal(res)
	if err == nil {
		return answer, nil
	} else {
		return nil, err
	}
}

func ReadJsonFromFile(fname string, config interface{}) error {
	content, err := ioutil.ReadFile(fname)
	if err != nil {
		return err
	}

	err = json.Unmarshal(content, config)
	if err != nil {
		return err
	}

	return nil
}
