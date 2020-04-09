{{/* Generate configmaps */}}
{{- define "asapo.configmap-fromfile" }}
apiVersion: v1
kind: ConfigMap
metadata:
  name: {{ .service.serviceName }}-config
data:
  {{ .service.serviceName }}.json:  {{ tpl (.Files.Get (printf "configs/%s.json" .service.serviceName)) . | quote }}

{{- if .service.sidecarLogs }}
---
apiVersion: v1
kind: ConfigMap
metadata:
  name: {{ .service.serviceName }}-fluentd-config
data:
  asapo-fluentd.conf:  {{ tpl (.Files.Get  ("configs/asapo-fluentd.conf")) . | quote }}
{{- end }}

{{- end }}


