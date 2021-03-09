package authorization

type TokenRequest struct {
	Subject    map[string]string
	DaysValid  int
	AccessType string
}

type TokenResponce struct {
	Token      string
	Sub        string
	AccessType string
	Expires    string
}
