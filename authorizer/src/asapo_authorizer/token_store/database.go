package token_store

import "asapo_common/utils"


const KAdminDb = "asapo_admin"
const KTokens = "tokens"
const KRevokedTokens = "revoked_tokens"

type Request struct {
	DbName         string
	Collection     string
	Op             string
}

type TokenRecord struct {
	Id string `bson:"_id"`
	*utils.CustomClaims
	Token string
	Revoked bool
}

type IdRecord struct {
	Id string `bson:"_id"`
}

type Agent interface {
	ProcessRequest(request Request, extraParams ...interface{}) ([]byte, error)
	Ping() error
	Connect(string) error
	Close()
}

type DBError struct {
	Code    int
	Message string
}

func (err *DBError) Error() string {
	return err.Message
}

func GetStatusCodeFromError(err error) int {
	err_db, ok := err.(*DBError)
	if ok {
		return err_db.Code
	} else {
		return utils.StatusServiceUnavailable
	}
}
