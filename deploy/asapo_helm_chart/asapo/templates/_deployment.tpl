{{/* Generate add fluentd sidecar */}}
{{- define "asapo.fluentd.container" }}
- name: fluentd
  image: "yakser/fluentd_elastic"
  command: ["fluentd"]
  args: ["-c", "/fluentd/etc/asapo-fluentd.conf"]
  volumeMounts:
    - mountPath: "/fluentd/etc"
      name: fluentd-config
    - mountPath: /var/log/containers
      name: logs
    - mountPath: /var
      name: var
{{- end }}

{{/* Generate add fluentd sidecar */}}
{{- define "asapo.fluentd.volumes" }}
- name: fluentd-config
  configMap:
    name: {{ .serviceName }}-fluentd-config
- name: logs
  hostPath:
    path: /var/log/containers
    type: Directory
- name: var
  hostPath:
    path: /var
    type: Directory
{{- end }}
