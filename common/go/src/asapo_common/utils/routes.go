package utils

import (
	"net/http"
	"github.com/gorilla/mux"
	"strings"
)

type Routes []Route

type Route struct {
	Name        string
	Method      string
	Pattern     string
	HandlerFunc http.HandlerFunc
}

func NewRouter(listRoutes Routes) *mux.Router {
	router := mux.NewRouter().UseEncodedPath()
	for _, route := range listRoutes {
		router.
			Methods(route.Method).
			Path(route.Pattern).
			Name(route.Name).
			Handler(route.HandlerFunc)
		// allow routes without trailing slash
		if strings.HasSuffix(route.Pattern, "/") {
			router.
				Methods(route.Method).
				Path(strings.TrimSuffix(route.Pattern, "/")).
				Name(route.Name + "_noslash").
				Handler(route.HandlerFunc)
		}
	}
	return router
}
