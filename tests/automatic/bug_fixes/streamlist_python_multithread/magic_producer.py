from concurrent.futures import ThreadPoolExecutor
import threading
import asapo_producer
import asapo_consumer
from time import sleep
from datetime import datetime
import sys

endpoint, beamtime, token = sys.argv[1:]


def dummy_producer(data_source):
    def callback(header, err):
        if err is not None:
            print("could not sent: ", header, err)
    #        else:
    #            print("successfuly sent: ", header)
    producer = asapo_producer.create_producer(endpoint, 'processed', beamtime, 'auto', data_source, '', 5,
                                              60000)  # source type 'processed' to write to the core filesystem
    producer.set_log_level("none")
    consumer = asapo_consumer.create_consumer(endpoint, "", False, beamtime, data_source, token, 3000)

    for j in range(5):
        stream = datetime.now().strftime('%Y%m%dT_%H%M%S')
        for i in range(5):
            producer.send(i + 1, data_source + "_" + stream + "_" + str(i), None,
                          callback=callback,
                          ingest_mode=asapo_producer.INGEST_MODE_TRANSFER_METADATA_ONLY,
                          stream=stream)
            sleep(0.6)
        producer.send_stream_finished_flag(stream=stream, last_id=i + 1)
        print(j + 1, ": number of streams", data_source + ": ", len(consumer.get_stream_list()))


def main():
    with ThreadPoolExecutor(max_workers=3) as executor:
        task1 = executor.submit(dummy_producer, "source_1")
        task2 = executor.submit(dummy_producer, "source_2")


if __name__ == '__main__':
    main()
