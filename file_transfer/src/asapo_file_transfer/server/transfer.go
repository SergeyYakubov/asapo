package server

import (
	log "asapo_common/logger"
	"asapo_common/structs"
	"asapo_common/utils"
	"asapo_common/version"
	"encoding/json"
	"errors"
	"net/http"
	"os"
	"path"
	"path/filepath"
)

type fileTransferRequest struct {
	Folder   string
	FileName string
}

func Exists(name string) bool {
	f, err := os.Open(name)
	defer f.Close()
	return err == nil
}

func checkClaim(r *http.Request, ver utils.VersionNum, request *fileTransferRequest) (int, error) {
	var extraClaim structs.FolderTokenTokenExtraClaim
	if err := utils.JobClaimFromContext(r, nil, &extraClaim); err != nil {
		return http.StatusInternalServerError, err
	}
	if ver.Id > 1 {
		request.Folder = extraClaim.RootFolder
		return http.StatusOK, nil
	}

	if extraClaim.RootFolder != request.Folder {
		err_txt := "access forbidden for folder " + request.Folder
		log.Error("cannot transfer file: " + err_txt)
		return http.StatusUnauthorized, errors.New(err_txt)
	}
	return http.StatusOK, nil
}

func checkFileExists(r *http.Request, name string) (int, error) {
	if !Exists(name) {
		err_txt := "file " + name + " does not exist or cannot be read"
		log.Error("cannot transfer file: " + err_txt)
		return http.StatusNotFound, errors.New(err_txt)
	}
	return http.StatusOK, nil

}

func checkRequest(r *http.Request, ver utils.VersionNum) (string, int, error) {
	var request fileTransferRequest
	err := utils.ExtractRequest(r, &request)
	if err != nil {
		return "", http.StatusBadRequest, err
	}

	if status, err := checkClaim(r, ver, &request); err != nil {
		return "", status, err
	}
	var fullName string
	if ver.Id == 1 { // protocol v0.1
		fullName = filepath.Clean(request.Folder + string(os.PathSeparator) + request.FileName)
	} else {
		fullName = filepath.Clean(request.Folder + string(os.PathSeparator) + request.FileName)
	}

	if status, err := checkFileExists(r, fullName); err != nil {
		return "", status, err
	}
	return fullName, http.StatusOK, nil
}

func serveFile(w http.ResponseWriter, r *http.Request, fullName string) {
	_, file := path.Split(fullName)
	w.Header().Set("Content-Disposition", "attachment; filename=\""+file+"\"")

	log.WithFields(map[string]interface{}{
		"name": fullName,
	}).Debug("transferring file")

	http.ServeFile(w, r, fullName)
}

func serveFileSize(w http.ResponseWriter, r *http.Request, fullName string) {
	var fsize struct {
		FileSize int64 `json:"file_size"`
	}

	fi, err := os.Stat(fullName)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusBadRequest)
		log.Error("error getting file size for " + fullName + ": " + err.Error())
	}

	log.WithFields(map[string]interface{}{
		"name": fullName,
		"size": fi.Size(),
	}).Debug("sending file size")

	fsize.FileSize = fi.Size()
	b, _ := json.Marshal(&fsize)
	w.Write(b)
}

func checkFtsApiVersion(w http.ResponseWriter, r *http.Request) (utils.VersionNum, bool) {
	return utils.PrecheckApiVersion(w, r, version.GetFtsApiVersion())
}

func routeFileTransfer(w http.ResponseWriter, r *http.Request) {
	ver, ok := checkFtsApiVersion(w, r)
	if !ok {
		return
	}

	fullName, status, err := checkRequest(r, ver)
	if err != nil {
		utils.WriteServerError(w, err, status)
		return
	}

	sizeonly := r.URL.Query().Get("sizeonly")
	if sizeonly != "true" {
		serveFile(w, r, fullName)
	} else {
		serveFileSize(w, r, fullName)
	}

}
