package request_handler

import (
	"asapo_common/utils"
	"asapo_discovery/common"
	"errors"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/client-go/kubernetes"
	"k8s.io/client-go/rest"
	"k8s.io/client-go/tools/clientcmd"
	"os"
	"path/filepath"
	"sort"
	"strconv"
)

type KubernetesRequestHandler struct {
	MaxConnections int
	client         *kubernetes.Clientset
	staticHandler  *StaticRequestHandler
	namespace string
}

func (rh *KubernetesRequestHandler) GetServices(name string, use_ib bool) ([]string, error) {
	pods, err := rh.client.CoreV1().Pods(rh.namespace).List(metav1.ListOptions{LabelSelector:"app="+name,FieldSelector:"status.phase=Running"})

	var result = make([]string, 0)

	services, err := rh.client.CoreV1().Services(rh.namespace).List(metav1.ListOptions{FieldSelector:"metadata.name="+name})
	if err != nil {
		return []string{},err
	}

	if len(services.Items) != 1 || len(services.Items[0].Spec.Ports)!=1 {
		return []string{},errors.New("cannot find kubernetes service or port")
	}

	port := strconv.Itoa(int(services.Items[0].Spec.Ports[0].NodePort))
	for _,pod:=range pods.Items {
		result = append(result, pod.Status.HostIP+":"+port)
	}
	sort.Strings(result)
	return result, nil
}

func (rh *KubernetesRequestHandler) GetSingleService(name string) ([]byte, error) {
	if len(rh.staticHandler.singleServices[name]) > 0 {
		return rh.staticHandler.GetSingleService(name)
	}

	if rh.client == nil {
		return nil, errors.New("Kubernetes client not initialized")
	}
	response, err := rh.GetServices(name, false)
	if err != nil {
		return nil, err
	}
	size := len(response)
	if size == 0 {
		return []byte(""), nil
	} else {
		return []byte(response[counter.Next(size)]), nil
	}
	return nil, nil

}

func (rh *KubernetesRequestHandler) GetReceivers(use_ib bool) ([]byte, error) {
	if len(rh.staticHandler.receiverResponce.Uris)>0 {
		return rh.staticHandler.GetReceivers(false)
	}

	var response Responce
	response.MaxConnections = rh.MaxConnections
	if (rh.client == nil) {
		return nil, errors.New("kubernetes client not initialized")
	}
	var err error
	response.Uris, err = rh.GetServices("asapo-receiver",use_ib)
	if err != nil {
		return nil, err
	}
	return utils.MapToJson(&response)
}

func (rh *KubernetesRequestHandler) createExternalConfig(settings common.Settings) (config *rest.Config, err error) {
	var kubeconfig string
	if len(settings.Kubernetes.ConfigFile) == 0 {
		if home := homeDir(); home != "" {
			kubeconfig = filepath.Join(home, ".kube", "config")
		} else {
			return nil,errors.New("cannot set default kubeconfig file")
		}
	} else {
		kubeconfig = settings.Kubernetes.ConfigFile
	}

	config, err = clientcmd.BuildConfigFromFlags("", kubeconfig)
	return config,err
}

func (rh *KubernetesRequestHandler) createInternalConfig() (config *rest.Config, err error) {
	return rest.InClusterConfig()
}

func (rh *KubernetesRequestHandler) Init(settings common.Settings) (err error) {
	rh.staticHandler = new(StaticRequestHandler)
	rh.staticHandler.Init(settings)
	rh.MaxConnections = settings.Receiver.MaxConnections

	var config *rest.Config
	if  settings.Kubernetes.Mode=="external" {
		config, err = rh.createExternalConfig(settings)
	} else {
		config, err = rh.createInternalConfig()
	}
	if err != nil {
		return err
	}

	rh.client, err = kubernetes.NewForConfig(config)
	rh.namespace = settings.Kubernetes.Namespace
	return err
}

func homeDir() string {
	if h := os.Getenv("HOME"); h != "" {
		return h
	}
	return os.Getenv("USERPROFILE") // windows
}
