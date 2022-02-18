elk_logs = true
perf_monitor = true

asapo_docker_repository = "yakser"
asapo_imagename_suffix = ""
asapo_image_tag = ""

influxdb_version="1.8.4"

service_dir="/gpfs/asapo/shared/service_dir"
online_dir="/beamline"
offline_dir="/asap3"
mongo_dir="/gpfs/asapo/shared/service_dir/mongodb"
asapo_user="35841:1000"
job_scripts_dir="/gpfs/asapo/shared/terraform"

ldap_uri="ldap://it-ldap-slave.desy.de:1389"

receiver_total_memory_size = 35000
receiver_dataserver_cache_size = 30 #gb
receiver_receive_to_disk_threshold = 50 # mb
receiver_dataserver_nthreads = 8
receiver_network_modes = "tcp"
receiver_kafka_enabled = false
receiver_kafka_metadata_broker_list = ""

grafana_total_memory_size = 2000
influxdb_total_memory_size = 2000
fluentd_total_memory_size = 1000
elasticsearch_total_memory_size = 3000
kibana_total_memory_size = 1000
mongo_total_memory_size = 40000
authorizer_total_memory_size = 20000
discovery_total_memory_size = 512

n_receivers = 2
n_brokers = 2
n_fts = 2
