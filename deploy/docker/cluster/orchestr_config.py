import time
import argparse
from string import Template
import socket
import json
import os

def is_server(ip,server_names):
    servers = json.loads(server_names)
    if len(servers) == 1:
        return "true"
    for server in json.loads(server_names):
        try:
            server_ip = socket.gethostbyname(server)
        except:
            server_ip = server
        if ip == server_ip:
            return "true"
    return "false"

def my_get_env(name,default):
    res = os.getenv(name)
    if res == None or res=='':
        res = default
        if default=='':
            raise Exception(name + ' not set')
    return res

def set_parameters():
    d = {}
    try:
        my_ip = socket.gethostbyname(socket.gethostname())
    except:
        print ("cannot define own ip")
        my_ip = "127.0.0.1"
    d['advertise_ip']=my_get_env('ADVERTISE_IP',my_ip)
    d['n_servers']=my_get_env('N_SERVERS',1)
    d['server_adresses']=my_get_env('SERVER_ADRESSES','["'+socket.gethostname()+'"]')
    d['is_server']=is_server(d['advertise_ip'],d['server_adresses'])
    d['ib_address']=my_get_env('IB_ADDRESS',"none")
    d['nomad_alloc_dir']=my_get_env('NOMAD_ALLOC_DIR','')
    d['recursors']=my_get_env('RECURSORS','["8.8.8.8"]')
    return d

def process_file(file_in,file_out):
    print ("processing " + file_in+" to "+file_out)
    filein = open(file_in)
    src = Template(filein.read())
    d = set_parameters()
    print d

    with open(file_out, "w") as out:
        out.write(src.substitute(d))

if __name__ == '__main__':

    parser = argparse.ArgumentParser()

    parser.add_argument('--input-files', action='store', dest='input_files',nargs="*",required=True)
    parser.add_argument('--output-files', action='store', dest='output_files',nargs="*",required=True)

    args = parser.parse_args()

    pairs = list(zip(args.input_files, args.output_files))

    for pair in pairs:
        process_file(pair[0],pair[1])

    print ("finished configuring nomad and consul")
    time.sleep(1)

