{{/* Generate ingres */}}
{{- define "asapo.ingress" }}
apiVersion: networking.k8s.io/v1beta1
kind: Ingress
metadata:
  name: ingress-{{ .service.serviceName }}
  annotations:
    kubernetes.io/ingress.class: "nginx"
    nginx.ingress.kubernetes.io/rewrite-target: /$2
    nginx.ingress.kubernetes.io/whitelist-source-range: 131.169.0.0/16
spec:
  rules:
    - host: "*.desy.de"
      http:
        paths:
          - path: /{{ .Release.Namespace }}/{{ .service.serviceName }}(/|$)(.*)
            backend:
              serviceName: {{ .service.serviceName }}
              servicePort: {{ .service.port }}
{{- end }}
