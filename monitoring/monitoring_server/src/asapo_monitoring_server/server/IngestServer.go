package server

import (
	pb "asapo_common/generated_proto"
	log "asapo_common/logger"
	"context"
	influxdb2 "github.com/influxdata/influxdb-client-go/v2"
	"github.com/influxdata/influxdb-client-go/v2/api"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	"time"
)

type IngestServer struct {
	pb.UnimplementedAsapoMonitoringIngestServiceServer

	settings    Settings
	dbWriterApi api.WriteAPI
}

func (s *IngestServer) InsertReceiverDataPoints(ctx context.Context, data *pb.ReceiverDataPointContainer) (*pb.Empty, error) {
	log.Debug("Received InsertReceiverDataPoints")

	if data.ReceiverName == "" {
		return nil, status.Errorf(codes.InvalidArgument, "ReceiverName must be filled")
	}

	timestamp := time.Unix(0, int64(data.TimestampMs)*int64(time.Millisecond))
	for _, dataPoint := range data.GroupedP2RTransfers {
		count := dataPoint.FileCount
		if count == 0 { // fix div by 0
			count = 1
		}

		p := influxdb2.NewPoint(dbMeasurementFileInput,
			map[string]string{
				"receiverName":       data.ReceiverName,
				"pipelineStepId":     dataPoint.PipelineStepId,
				"producerInstanceId": dataPoint.ProducerInstanceId,
				"beamtime": dataPoint.Beamtime,
				"source":   dataPoint.Source,
				"stream":   dataPoint.Stream,
			},
			map[string]interface{}{
				"totalInputFileSize":       int64(dataPoint.TotalFileSize),
				"avgTransferReceiveTimeUs": int64(dataPoint.TotalTransferReceiveTimeInMicroseconds / count),
				"avgWriteIoTimeUs":         int64(dataPoint.TotalWriteIoTimeInMicroseconds / count),
				"avgDbTimeUs":              int64(dataPoint.TotalDbTimeInMicroseconds / count),
				"receiverFileCount":        int64(dataPoint.FileCount),
			},
			timestamp,
		)

		s.dbWriterApi.WritePoint(p)
	}

	for _, dataPoint := range data.GroupedRds2CTransfers {
		count := dataPoint.Hits
		if count == 0 { // fix div by 0
			count = 1
		}
		p := influxdb2.NewPoint(dbMeasurementRdsFileRequests,
			map[string]string{
				"receiverName":       data.ReceiverName,
				"pipelineStepId":     dataPoint.PipelineStepId,
				"consumerInstanceId": dataPoint.ConsumerInstanceId,

				"beamtime": dataPoint.Beamtime,
				"source":   dataPoint.Source,
				"stream":   dataPoint.Stream,
			},
			map[string]interface{}{
				"totalRdsHits":               int64(dataPoint.Hits),
				"totalRdsMisses":             int64(dataPoint.Misses),
				"totalRdsOutputFileSize":     int64(dataPoint.TotalFileSize),
				"avgRdsOutputTransferTimeUs": int64(dataPoint.TotalTransferSendTimeInMicroseconds / count),
			},
			timestamp,
		)

		s.dbWriterApi.WritePoint(p)
	}

	for _, dataPoint := range data.GroupedMemoryStats {
		p := influxdb2.NewPoint(dbMeasurementRdsCacheMemoryUsage,
			map[string]string{
				"receiverName": data.ReceiverName,

				"beamtime": dataPoint.Beamtime,
				"source":   dataPoint.Source,
				"stream":   dataPoint.Stream,
			},
			map[string]interface{}{
				"rdsCacheUsedBytes":  int64(dataPoint.UsedBytes),
				"rdsCacheTotalBytes": int64(dataPoint.TotalBytes),
			},
			timestamp,
		)

		s.dbWriterApi.WritePoint(p)
	}

	s.dbWriterApi.Flush()

	return &pb.Empty{}, nil
}

func (s *IngestServer) InsertBrokerDataPoints(ctx context.Context, data *pb.BrokerDataPointContainer) (*pb.Empty, error) {
	log.Debug("Received InsertBrokerDataPoints")

	if data.BrokerName == "" {
		return nil, status.Errorf(codes.InvalidArgument, "BrokerName must be filled")
	}

	timestamp := time.Unix(0, int64(data.TimestampMs)*int64(time.Millisecond))
	for _, dataPoint := range data.GroupedBrokerRequests {
		p := influxdb2.NewPoint(dbMeasurementBrokerFileRequests,
			map[string]string{
				"brokerName":         data.BrokerName,
				"pipelineStepId":     dataPoint.PipelineStepId,
				"consumerInstanceId": dataPoint.ConsumerInstanceId,

				"brokerCommand": dataPoint.Command,

				"beamtime": dataPoint.Beamtime,
				"source":   dataPoint.Source,
				"stream":   dataPoint.Stream,
			},
			map[string]interface{}{
				"requestedFileCount":     int64(dataPoint.FileCount),
				"totalRequestedFileSize": int64(dataPoint.TotalFileSize),
			},
			timestamp,
		)

		s.dbWriterApi.WritePoint(p)
	}
	s.dbWriterApi.Flush()
	return &pb.Empty{}, nil
}

func (s *IngestServer) InsertFtsDataPoints(ctx context.Context, data *pb.FtsToConsumerDataPointContainer) (*pb.Empty, error) {
	log.Debug("Received InsertFtsDataPoints")

	if data.FtsName == "" {
		return nil, status.Errorf(codes.InvalidArgument, "FtsName must be filled")
	}

	timestamp := time.Unix(0, int64(data.TimestampMs)*int64(time.Millisecond))
	for _, dataPoint := range data.GroupedFdsTransfers {

		count := dataPoint.FileCount
		if count == 0 { // fix div by 0
			count = 1
		}

		p := influxdb2.NewPoint(dbMeasurementFtsTransfers,
			map[string]string{
				"ftsName":            data.FtsName,
				"pipelineStepId":     dataPoint.PipelineStepId,
				"consumerInstanceId": dataPoint.ConsumerInstanceId,

				"beamtime": dataPoint.Beamtime,
				"source":   dataPoint.Source,
				"stream":   dataPoint.Stream,
			},
			map[string]interface{}{
				"ftsFileCount":                int64(dataPoint.FileCount),
				"totalFtsTransferredFileSize": int64(dataPoint.TotalFileSize),
				"avgTransferSendTimeUs":       int64(dataPoint.TotalTransferSendTimeInMicroseconds / count),
			},
			timestamp,
		)

		s.dbWriterApi.WritePoint(p)
	}

	s.dbWriterApi.Flush()
	return &pb.Empty{}, nil
}
