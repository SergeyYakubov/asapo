#ifndef HIDRA2_COMMON__NET_SERVER_H_H
#define HIDRA2_COMMON__NET_SERVER_H_H

namespace HIDRA2 {
    class NetClientPeer {
        friend NetServer;
    private:
        int fd;
        bool is_accepted;
    public:

    };

    typedef std::function<void(NetClientPeer& peer)> NetworkNewPeerHandler;
    typedef std::function<void(NetClientPeer& peer, void* data, size_t data_length)> NetworkPackageHandler;

    class NetServer {
    public:
        void bind(std::string address, ushort port);
        void accept(NetClientPeer& peer, NetworkPackageHandler handler);
    };
}

#endif //HIDRA2_COMMON__NET_SERVER_H_H
