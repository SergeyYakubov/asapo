import asapo_producer

# callback snippet_start
def callback(payload,err):
    if err is not None and not isinstance(err, asapo_producer.AsapoServerWarning):
        # the data was not sent. Something is terribly wrong.
        print("could not send: ",payload,err)
    elif err is not None:
        # The data was sent, but there was some unexpected problem, e.g. the file was overwritten.
        print("sent with warning: ",payload,err)
    else:
        # all fine
        print("successfuly sent: ",payload)
# callback snippet_end

# create snippet_start
endpoint = "localhost:8400"
beamtime = "asapo_test"

producer = asapo_producer \
                .create_producer(endpoint,
                                 'processed',    # should be 'processed' or 'raw', 'processed' writes to the core FS
                                 beamtime,       # the folder should exist
                                 'auto',         # can be 'auto', if beamtime_id is given
                                 'test_source',  # source
                                 '',             # athorization token
                                 1,              # number of threads. Increase, if the sending speed seems slow
                                 60000)          # timeout. Do not change.

producer.set_log_level("error") # other values are "warning", "info" or "debug".
# create snippet_end

# send snippet_start
# we are sending a message with with index 1 to the default stream. Filename must start with processed/
producer.send(1,                     # message number. Should be unique and ordered.
              "processed/test_file", # name of the file. Should be unique, or it will be overwritten
              b"hello",              # binary data
              callback = callback)   # callback
# send snippet_end
# send data in loop

# add the following at the end of the script

# finish snippet_start
producer.wait_requests_finished(2000) # will synchronously wait for all the data to be sent.
                                      # Use it when no more data is expected.

# you may want to mark the stream as finished
producer.send_stream_finished_flag("default", # name of the stream. If you didn't specify the stream in 'send', it would be 'default'
                                   1)         # the number of the last message in the stream
# finish snippet_end
