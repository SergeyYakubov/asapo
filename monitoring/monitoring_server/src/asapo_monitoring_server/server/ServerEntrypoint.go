package server

import (
	pb "asapo_common/generated_proto"
	log "asapo_common/logger"
	"context"
	influxdb2 "github.com/influxdata/influxdb-client-go/v2"
	"google.golang.org/grpc"
	"net"
	"strconv"
)

func Start(settings Settings) {
	lis, err := net.Listen("tcp", ":"+strconv.Itoa(int(settings.ServerPort)))
	if err != nil {
		log.Fatal("failed to listen: ", err)
	}

	influxClient := influxdb2.NewClient(settings.InfluxDbUrl, ""/*settings.InfluxDb2AuthToken*/)
	// influxClient.BucketsAPI().CreateBucketWithName(context.Background(), domain.Organization{}, "bucketName", domain.RetentionRules{})

	queryServer := QueryServer{}
	ingestServer := IngestServer{}

	dbQueryApi := influxClient.QueryAPI(""/*settings.InfluxDb2Org*/)
	queryServer.dbQueryApi = dbQueryApi
	queryServer.settings = settings

	dbWriterApi := influxClient.WriteAPI(""/*settings.InfluxDb2Org*/, settings.InfluxDbDatabase)
	ingestServer.dbWriterApi = dbWriterApi
	ingestServer.settings = settings

	grpcServer := grpc.NewServer()
	pb.RegisterAsapoMonitoringQueryServiceServer(grpcServer, &queryServer)
	pb.RegisterAsapoMonitoringIngestServiceServer(grpcServer, &ingestServer)
	log.Info("server listening at ", lis.Addr())

	// Check connection
	_, err = queryServer.GetMetadata(context.TODO(), &pb.Empty{})
	if err != nil {
		log.Fatal("dummy database request failed: ", err)
	}

	// Start net server and wait
	if err = grpcServer.Serve(lis); err != nil {
		log.Fatal("failed to serve: ", err)
	}
}
