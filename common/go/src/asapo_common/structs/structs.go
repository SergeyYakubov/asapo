package structs

type FolderTokenTokenExtraClaim struct {
	RootFolder string
}

type AccessTokenExtraClaim struct {
	AccessType string
}

type IntrospectTokenRequest struct {
	Token      string
}

type IntrospectTokenResponse struct {
	Sub        string
	AccessType string
}

type IssueTokenRequest struct {
	Subject    map[string]string
	DaysValid  int
	AccessType string
}

type IssueTokenResponse struct {
	Token      string
	Sub        string
	AccessType string
	Expires    string
}
