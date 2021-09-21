package server

const (
	// InfluxDb bucket definition
	// TAGS
	// > Values

	// definition: sourceMeta = beamtime, source, stream

	// From receiver when files are inserted into ASAPO:
	// [sourceMeta], receiverName, pipelineStepId, producerInstanceId
	// > totalInputFileSize, avgTransferReceiveTimeUs, avgWriteIoTimeUs, avgDbTimeUs, receiverFileCount
	dbMeasurementFileInput = "fileInput"

	// From Broker when requests are handled:
	// [sourceMeta], brokerName, pipelineStepId, consumerInstanceId, command
	// > totalRequestedFileSize, requestedFileCount
	dbMeasurementBrokerFileRequests = "brokerRequests"

	// From receiver data service when files are requested
	// [sourceMeta], receiverName, pipelineStepId, consumerInstanceId
	// > (totalRdsOutputFileSize, avgRdsOutputTransferTimeUs)
	dbMeasurementRdsFileRequests = "rdsRequests"

	// From receiver data service when files are requested
	// [sourceMeta], receiverName
	// > rdsCacheUsedBytes, rdsCacheTotalBytes
	dbMeasurementRdsCacheMemoryUsage = "rdsCacheMemoryUsage"

	// From file transfer service when files are transferred
	// [sourceMeta], ftsName, pipelineStepId, consumerInstanceId
	// > ftsFileCount, totalFtsTransferredFileSize, avgTransferSendTimeUs
	dbMeasurementFtsTransfers = "ftsTransfers"
)

type Settings struct {
	ThisClusterName string // e.g. "test-cluster"

	ServerPort uint16 // e.g. "50051"
	LogLevel   string // e.g. "info"

	InfluxDbUrl       string // e.g. "http://127.0.0.1:8086"
	InfluxDbDatabase  string // e.g. "asapo-monitoring"
}
