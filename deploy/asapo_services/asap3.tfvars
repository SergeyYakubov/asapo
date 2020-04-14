elk_logs = true

asapo_image_tag = "develop"

service_dir="/gpfs/asapo/shared/service_dir"
online_dir="/beamline"
offline_dir="/asap3"
mongo_dir="/gpfs/asapo/shared/service_dir/mongodb"
asapo_user="35841:1000"
job_scripts_dir="/gpfs/asapo/shared/terraform"

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
n_fts = 1


