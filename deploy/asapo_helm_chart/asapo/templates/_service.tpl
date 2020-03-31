{{/* Generate service */}}
{{- define "asapo.service" }}
apiVersion: v1
kind: Service
metadata:
  name: {{ .service.serviceName }}
spec:
  type: {{ if .service._exposeServiceExtrernally }}NodePort{{ else }}ClusterIP{{ end }}
  ports:
    - protocol: TCP
      port: {{ .service.port }}
  selector:
    app: {{ .service.appName | default .service.serviceName }}
{{- end }}
