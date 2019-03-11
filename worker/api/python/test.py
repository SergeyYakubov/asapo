import asapo_worker
import sys


broker,err = asapo_worker.CreateServerBroker("1","1","1","1")

if not broker:
    print("Cannot create broker: "+err)
    sys.exit(1)

print broker.get_data()


