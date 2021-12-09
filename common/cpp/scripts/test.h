  #MOCK_CONST_METHOD0(GetFileName, std::string()); 
 MOCK_CONST_METHOD0(GetOriginUri_t, 
  const std::string & ());
MOCK_CONST_METHOD0(GetOriginHost, const std::string & ());
MOCK_CONST_METHOD0(GetDataSize, uint64_t());
MOCK_CONST_METHOD0(GetDataID, uint64_t());
MOCK_CONST_METHOD0(GetSlotId, uint64_t());
MOCK_CONST_METHOD0(GetData, void* ());
MOCK_CONST_METHOD0(GetBeamtimeId, const std::string & ());
MOCK_CONST_METHOD0(GetDataSource, const std::string & ());
MOCK_METHOD0(SetAlreadyProcessedFlag, void());
MOCK_METHOD2(SetResponseMessage, void(std::string, ResponseMessageType));
MOCK_CONST_METHOD0(GetResponseMessage, const std::string & ());
MOCK_CONST_METHOD1(NewThread_t, std::thread * (std::function<void()> function));
MOCK_CONST_METHOD2(InetAcceptConnection_t, std::tuple<std::string, SocketDescriptor>* (SocketDescriptor socket_fd,
      ErrorInterface** err));
 dfgd
 gds
  gdfsg

