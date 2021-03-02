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

func DeepCopy(a, b interface{}) {
	byt, _ := json.Marshal(a)
	json.Unmarshal(byt, b)
}


func GetInt64FromMap(s map[string]interface{}, name string) (int64,bool) {
	val, ok := InterfaceToInt64(s[name])
	if ok {
		return val,true
	} else {
		return -1, false
	}
}


func InterfaceToInt64(val interface{}) (int64, bool) {
	val64, ok := val.(int64)
	var valf64 float64
	if !ok { // we need this (at least for tests) since by default values are float in mongo
		valf64, ok = val.(float64)
		val64 = int64(valf64)
	}
	return val64, ok
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
