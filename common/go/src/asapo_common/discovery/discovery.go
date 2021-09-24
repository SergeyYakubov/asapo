package discovery

import (
	"io/ioutil"
	"net/http"
)

type DiscoveryAPI struct {
	client  *http.Client
	baseURL string
}

func (api *DiscoveryAPI) GetMongoDbAddress() (string, error) {
	resp, err := api.client.Get(api.baseURL + "/asapo-mongodb")
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	return string(body), err
}

func CreateDiscoveryService(client *http.Client,uri string) DiscoveryAPI{
	return DiscoveryAPI{client, uri}
}