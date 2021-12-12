package token_store

import (
	"asapo_authorizer/common"
	"asapo_common/discovery"
	log "asapo_common/logger"
	"asapo_common/utils"
	"errors"
	"net/http"
	"sync"
	"time"
)

type Store interface {
	Init(db Agent) error
	AddToken(token TokenRecord) error
	RevokeToken(token string, id string) (TokenRecord, error)
	GetTokenList() ([]TokenRecord, error)
	IsTokenRevoked(tokenId string) (bool, error)
	Close()
}

type TokenStore struct {
	db               Agent
	revokedTokenList struct {
		tokens     []string
		lock       sync.RWMutex
		lastUpdate time.Time
		lastError  error
	}
	stopchan chan bool
}

var discoveryService discovery.DiscoveryAPI

func (store *TokenStore) reconnectDb() (dbaddress string, err error) {
	if store.db == nil {
		return "", errors.New("token_store not initialized")
	}
	store.db.Close()
	return store.initDB()
}

func (store *TokenStore) initDB() (dbaddress string, err error) {
	dbaddress = common.Settings.DatabaseServer
	if common.Settings.DatabaseServer == "auto" {
		dbaddress, err = discoveryService.GetMongoDbAddress()
		if err != nil {
			return "", err
		}
		if dbaddress == "" {
			return "", errors.New("no token_store servers found")
		}
		log.WithFields(map[string]interface{}{"address": dbaddress}).Debug("found mongodb server")
	}
	return dbaddress, store.db.Connect(dbaddress)

}

func (store *TokenStore) reconnectIfNeeded(db_error error) {
	code := GetStatusCodeFromError(db_error)
	if code != utils.StatusServiceUnavailable {
		return
	}

	if dbaddress, err := store.reconnectDb(); err != nil {
		log.Error("cannot reconnect to database: " + err.Error())
	} else {
		log.WithFields(map[string]interface{}{"address":dbaddress}).Debug("reconnected to database")
	}
}

func createDiscoveryService() {
	discoveryService = discovery.CreateDiscoveryService(&http.Client{}, "http://"+common.Settings.DiscoveryServer)
}

func (store *TokenStore) Init(db Agent) error {
	if db == nil {
		store.db = new(Mongodb)
	} else {
		store.db = db
	}
	createDiscoveryService()
	_, err := store.initDB()
	store.stopchan = make(chan bool)
	if common.Settings.UpdateRevokedTokensIntervalSec > 0 {
		go store.loopGetRevokedTokens()
	}
	return err
}

func (store *TokenStore) processError(err error) error {
	if err != nil {
		go store.reconnectIfNeeded(err)
	}
	return err
}

func (store *TokenStore) AddToken(token TokenRecord) error {
	_, err := store.db.ProcessRequest(Request{
		DbName:     KAdminDb,
		Collection: KTokens,
		Op:         "create_record",
	}, &token)
	return store.processError(err)
}

func (store *TokenStore) findToken(token string, id string) (TokenRecord, error) {
	q := map[string]interface{}{}
	if token != "" {
		q["token"] = token
	} else {
		q["_id"] = id
	}
	var record TokenRecord
	_, err := store.db.ProcessRequest(Request{
		DbName:     KAdminDb,
		Collection: KTokens,
		Op:         "read_record",
	}, q, &record)
	if err != nil {
		return TokenRecord{}, err
	}
	if record.Revoked {
		return TokenRecord{}, errors.New("token already revoked")
	}
	return record, nil
}

func (store *TokenStore) updateTokenStatus(token *TokenRecord) error {
	_, err := store.db.ProcessRequest(Request{
		DbName:     KAdminDb,
		Collection: KTokens,
		Op:         "update_record",
	}, token.Id, map[string]interface{}{"revoked": true}, false, token)
	return err
}

func (store *TokenStore) updateRevokedTokensList(tokenId string) error {
	idRec := IdRecord{tokenId}
	_, err := store.db.ProcessRequest(Request{
		DbName:     KAdminDb,
		Collection: KRevokedTokens,
		Op:         "create_record",
	}, &idRec)
	return err
}

func (store *TokenStore) RevokeToken(token string, id string) (TokenRecord, error) {
	tokenRecord, err := store.findToken(token, id)
	if err != nil {
		return TokenRecord{}, store.processError(err)
	}

	err = store.updateTokenStatus(&tokenRecord)
	if err != nil {
		return TokenRecord{}, store.processError(err)
	}

	err = store.updateRevokedTokensList(tokenRecord.Id)
	if err != nil {
		return TokenRecord{}, store.processError(err)
	}

	store.revokedTokenList.lock.Lock()
	defer store.revokedTokenList.lock.Unlock()
	store.revokedTokenList.tokens = append(store.revokedTokenList.tokens, tokenRecord.Id)
	return tokenRecord, nil
}

func (store *TokenStore) GetTokenList() ([]TokenRecord, error) {
	var res []TokenRecord
	_, err := store.db.ProcessRequest(Request{
		DbName:     KAdminDb,
		Collection: KTokens,
		Op:         "read_records",
	}, &res)
	return res, store.processError(err)
}

func (store *TokenStore) loopGetRevokedTokens() {
	next_update := 0
	for true {
		var res []IdRecord
		_, err := store.db.ProcessRequest(Request{
			DbName:     KAdminDb,
			Collection: KRevokedTokens,
			Op:         "read_records",
		}, &res)
		if err != nil {
			store.revokedTokenList.lock.Lock()
			store.revokedTokenList.lastError = err
			store.revokedTokenList.tokens = nil
			store.processError(err)
			store.revokedTokenList.lock.Unlock()
			next_update = 1
			log.Error("cannot get revoked tokens: " + err.Error())
		} else {
			//log.Debug("received revoked tokens list")
			next_update = common.Settings.UpdateRevokedTokensIntervalSec
			tokens := make([]string, len(res))
			for i, token := range res {
				tokens[i] = token.Id
			}
			store.revokedTokenList.lock.Lock()
			store.revokedTokenList.lastError = nil
			store.revokedTokenList.tokens = tokens
			store.revokedTokenList.lock.Unlock()
		}
		select {
		case <-store.stopchan:
			return
		case <-time.After(time.Duration(next_update) * time.Second):
			break
		}
	}
}

func (store *TokenStore) IsTokenRevoked(tokenId string) (bool, error) {
	tokens, err := store.getRevokedTokenIds()
	if err != nil {
		return true, err
	}
	return utils.StringInSlice(tokenId, tokens), nil
}

func (store *TokenStore) getRevokedTokenIds() ([]string, error) {
	store.revokedTokenList.lock.RLock()
	defer store.revokedTokenList.lock.RUnlock()
	if store.revokedTokenList.lastError != nil {
		return []string{}, store.revokedTokenList.lastError
	}
	//	res := make([]string, len(store.revokedTokenList.tokens))
	//	copy(res, store.revokedTokenList.tokens)
	//	return res,nil
	return store.revokedTokenList.tokens, nil
}

func (store *TokenStore) Close() {
	if common.Settings.UpdateRevokedTokensIntervalSec > 0 {
		store.stopchan <- true
	}
	if store.db != nil {
		store.db.Close()
	}
}
