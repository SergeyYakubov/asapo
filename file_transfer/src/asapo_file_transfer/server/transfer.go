package server

import (
	log "asapo_common/logger"
	"asapo_common/utils"
	"errors"
	"net/http"
	"os"
	"path"
)


type fileTransferRequest struct {
	Folder   string
	FileName string
}


func Exists(name string) bool {
	fi, err := os.Stat(name)
	return !os.IsNotExist(err) && !fi.IsDir()
}


func checkClaim(r *http.Request,request* fileTransferRequest) (int,error) {
	var extraClaim utils.FolderTokenTokenExtraClaim
	if err := utils.JobClaimFromContext(r, &extraClaim); err != nil {
		return http.StatusInternalServerError,err
	}
	if extraClaim.RootFolder!=request.Folder {
		err_txt := "access forbidden for folder "+request.Folder
		log.Error("cannot transfer file: "+err_txt)
		return http.StatusUnauthorized, errors.New(err_txt)
	}
	return http.StatusOK,nil
}

func checkFileExists(r *http.Request,name string) (int,error) {
	if !Exists(name) {
		err_txt := "file "+name+" does not exist"
		log.Error("cannot transfer file: "+err_txt)
		return http.StatusBadRequest,errors.New(err_txt)
	}
	return http.StatusOK,nil

}

func checkRequest(r *http.Request) (string,int,error) {
	var request fileTransferRequest
	err := utils.ExtractRequest(r,&request)
	if err != nil {
		return "",http.StatusBadRequest,err
	}

	if status,err := checkClaim(r,&request); err != nil {
		return "",status,err
	}
	fullName := request.Folder+string(os.PathSeparator)+request.FileName
	if status,err := checkFileExists(r,fullName); err != nil {
		return "",status,err
	}
	return fullName,http.StatusOK,nil
}

func serveFile(w http.ResponseWriter, r *http.Request, fullName string) {
	_, file := path.Split(fullName)
	w.Header().Set("Content-Disposition", "attachment; filename=\""+file+"\"")
	log.Debug("Transferring file " + fullName)
	http.ServeFile(w,r, fullName)
}

func routeFileTransfer(w http.ResponseWriter, r *http.Request) {
	fullName, status,err := checkRequest(r);
	if err != nil {
		utils.WriteServerError(w,err,status)
		return
	}
	serveFile(w,r,fullName)
}
