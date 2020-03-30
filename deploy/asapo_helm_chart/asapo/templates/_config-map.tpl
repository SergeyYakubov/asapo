{{/* Generate configmaps */}}
{{- define "asapo.configmap-fromfile" }}
apiVersion: v1
kind: ConfigMap
metadata:
  name: {{ .service.serviceName }}-config
data:
  {{ .service.serviceName }}.json:  {{ tpl (.Files.Get (printf "configs/%s.json" .service.serviceName)) . | quote }}
{{- end }}

