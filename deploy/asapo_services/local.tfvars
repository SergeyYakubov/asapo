elk_logs = false
perf_monitor = true

asapo_docker_repository = "yakser"
#asapo_imagename_suffix = ""
#asapo_image_tag = ""

influxdb_version="1.8.4"

service_dir="/var/tmp/asapo"
online_dir="/var/tmp/asapo/bl"
offline_dir="/var/tmp/asapo/core"
mongo_dir="/var/tmp/asapo/mongo"
asapo_user="26655:1000"
job_scripts_dir="/home/yakubov/projects/asapo/build/deploy/asapo_services/scripts"


ldap_uri="ldap://it-ldap-slave.desy.de:1389"

receiver_network_modes = "tcp"
receiver_kafka_enabled = false
receiver_kafka_metadata_broker_list=""

n_receivers = 1
n_brokers = 1
n_fts = 1
