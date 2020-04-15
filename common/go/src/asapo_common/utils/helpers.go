package utils

import (
	json "encoding/json"
	"io/ioutil"
	"strings"
	"errors"
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


func ReadStringsFromFile(fname string) ([]string, error) {
	content, err := ioutil.ReadFile(fname)
	if err != nil {
		return []string{},err
	}
	lines := strings.Split(string(content), "\n")

	return lines,nil
}


func ReadFirstStringFromFile(fname string) (string, error) {
	lines,err  := ReadStringsFromFile(fname)
	if err != nil {
		return "",err
	}

	if len(lines)==0 {
		return "",errors.New("empty file")
	}

	return lines[0],nil
}


func MapToStruct(m map[string]interface{}, val interface{}) error {
	tmp, err := json.Marshal(m)
	if err != nil {
		return err
	}
	err = json.Unmarshal(tmp, val)
	if err != nil {
		return err
	}
	return nil
}
