consul_dns_port = 53

elk_logs = true

service_dir="/bldocuments/support/asapo"
data_dir="/bldocuments/support/asapo/data"
mongo_dir="/bldocuments/support/asapo/mongodb"
asapo_user="994:989"
job_scripts_dir="/bldocuments/support/asapo/config/nomad_jobs/terraform_scripts"

receiver_total_memory_size = 35000
receiver_dataserver_cache_size = 30 #gb
receiver_receive_to_disk_threshold = 50 # mb
receiver_dataserver_nthreads = 8

grafana_total_memory_size = 2000
influxdb_total_memory_size = 2000
fluentd_total_memory_size = 1000
elasticsearch_total_memory_size = 3000
kibana_total_memory_size = 1000
mongo_total_memory_size = 20000
authorizer_total_memory_size = 512
discovery_total_memory_size = 512

n_receivers = 1
n_brokers = 1
