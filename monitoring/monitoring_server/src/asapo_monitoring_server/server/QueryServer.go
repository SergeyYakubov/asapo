package server

import (
	pb "asapo_common/generated_proto"
	log "asapo_common/logger"
	"context"
	"errors"
	"fmt"
	"github.com/influxdata/influxdb-client-go/v2/api"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
	"strconv"
	"strings"
)

type QueryServer struct {
	pb.UnimplementedAsapoMonitoringQueryServiceServer

	settings   Settings
	dbQueryApi api.QueryAPI
}

func (s *QueryServer) GetMetadata(ctx context.Context, _ *pb.Empty) (*pb.MetadataResponse, error) {
	result, err := s.dbQueryApi.Query(ctx, "import \"influxdata/influxdb/schema\"\n"+
		"schema.tagValues(bucket: \""+s.settings.InfluxDb2Bucket+"\", tag: \"beamtime\", predicate: (r) => true, start: -30d)")

	if err != nil {
		return nil, err
	}

	response := pb.MetadataResponse{}
	for result.Next() {
		response.AvailableBeamtimes = append(response.AvailableBeamtimes, result.Record().Values()["_value"].(string))
	}
	response.ClusterName = s.settings.ThisClusterName

	return &response, nil
}

func createFilterElement(tagName string, tagValue string) string {
	return "r[\"" + tagName + "\"] == \""+tagValue+"\""
}
type filterGenerator func(*pb.DataPointsQuery) string

func createFilter(query *pb.DataPointsQuery, generator filterGenerator) string {
	// always present in database: beamtime, source
	filter := " |> filter(fn: (r) => "
	filter += createFilterElement("beamtime", query.BeamtimeFilter)
	if query.SourceFilter != "" {
		filter += " and " + createFilterElement("source", query.SourceFilter)
	}

	filter += generator(query)

	filter += ")"

	return filter
}

func transferSpeedFilterGenerator(query *pb.DataPointsQuery) string {
	filter := ""

	if query.ReceiverFilter != "" {
		filter += " and " + createFilterElement("receiverName", query.ReceiverFilter)
	}
	if query.FromPipelineStepFilter != "" && query.ToPipelineStepFilter != "" {
		if query.PipelineQueryMode == pb.PipelineConnectionQueryMode_ExactPath {
			filter += " and (" +
				// From: receiver
				"(r._measurement == \"" + dbMeasurementFileInput + "\" and " + createFilterElement("pipelineStepId", query.FromPipelineStepFilter) + ")" +
				" or " +
				// To: broker or RDS or FTS
				"((r._measurement == \"" + dbMeasurementBrokerFileRequests + "\" or r._measurement == \"" + dbMeasurementRdsFileRequests + "\" or r._measurement == \"" + dbMeasurementFtsTransfers + "\") and " + createFilterElement("pipelineStepId", query.ToPipelineStepFilter) + ")" +
				")"
		} else if query.PipelineQueryMode == pb.PipelineConnectionQueryMode_JustRelatedToStep {
			// Direction is ignored here. Just check if piplineStepId is different
			if query.FromPipelineStepFilter == query.ToPipelineStepFilter {
				// from and to are the same
				filter += " and " + createFilterElement("pipelineStepId", query.FromPipelineStepFilter)
			} else {
				filter += " and (" +
					createFilterElement("pipelineStepId", query.FromPipelineStepFilter) +
					" or " +
					createFilterElement("pipelineStepId", query.ToPipelineStepFilter)
			}
		}
	}

	return filter
}

func transferRateFilterGenerator(query *pb.DataPointsQuery) string {
	filter := ""

	if query.ReceiverFilter != "" {
		filter += " and " + createFilterElement("receiverName", query.ReceiverFilter)
	}
	if query.FromPipelineStepFilter != "" {
		filter += " and " + createFilterElement("pipelineStepId", query.FromPipelineStepFilter)
	}

	return filter
}

func taskTimeFilterGenerator(query *pb.DataPointsQuery) string {
	filter := ""

	if query.ReceiverFilter != "" {
		filter += " and " + createFilterElement("receiverName", query.ReceiverFilter)
	}
	if query.PipelineQueryMode == pb.PipelineConnectionQueryMode_ExactPath {
		filter += " and (" +
			// From: receiver
			"(r._measurement == \"" + dbMeasurementFileInput + "\" and " + createFilterElement("pipelineStepId", query.FromPipelineStepFilter) + ")" +
			" or " +
			// To: RDS or FTS
			"((r._measurement == \"" + dbMeasurementRdsFileRequests + "\" or r._measurement == \"" + dbMeasurementFtsTransfers + "\") and " + createFilterElement("pipelineStepId", query.ToPipelineStepFilter) + ")" +
			")"
	} else if query.PipelineQueryMode == pb.PipelineConnectionQueryMode_JustRelatedToStep {
		// Direction is ignored here. Just check if piplineStepId is different
		if query.FromPipelineStepFilter == query.ToPipelineStepFilter {
			// from and to are the same
			filter += " and " + createFilterElement("pipelineStepId", query.FromPipelineStepFilter)
		} else {
			filter += " and (" +
				createFilterElement("pipelineStepId", query.FromPipelineStepFilter) +
				" or " +
				createFilterElement("pipelineStepId", query.ToPipelineStepFilter)
		}
	}

	return filter
}

func memoryUsageFilterGenerator(query *pb.DataPointsQuery) string {
	filter := ""

	if query.ReceiverFilter != "" {
		filter += " and " + createFilterElement("receiverName", query.ReceiverFilter)
	}

	return filter
}

// Checks if the string contains any characters that normally should not appear and
// that might interfere with InfluxDb2 Query
func doesStringContainsDangerousElements(input string) bool {
	// ref: https://github.com/influxdata/influxdb-client-js/blob/v1.15.0/packages/core/src/query/flux.ts#L40-L99
	return strings.Contains(input, "\"") ||
		strings.Contains(input, "\n") ||
		strings.Contains(input, "\r") ||
		strings.Contains(input, "\\") ||
		strings.Contains(input, "$") ||
		strings.Contains(input, "{") ||
		strings.Contains(input, "}")
}

func (s *QueryServer) GetDataPoints(ctx context.Context, query *pb.DataPointsQuery) (*pb.DataPointsResponse, error) {
	startTime := strconv.FormatUint(query.FromTimestamp, 10)
	endTime := strconv.FormatUint(query.ToTimestamp, 10)
	intervalInSec := 5


	if query.BeamtimeFilter == "" {
		return nil, status.Errorf(codes.InvalidArgument, "Beamtime is required")
	}

	// InfluxDB 2 does not really support input parameters. So the best attempt is to
	// if any from them include '"' or '\' throw an error
	if doesStringContainsDangerousElements(query.BeamtimeFilter) ||
		doesStringContainsDangerousElements(query.FromPipelineStepFilter) ||
		doesStringContainsDangerousElements(query.ToPipelineStepFilter) ||
		doesStringContainsDangerousElements(query.ReceiverFilter) ||
		doesStringContainsDangerousElements(query.SourceFilter) {
		return nil, errors.New("some input characters are not allowed")
	}

	response := pb.DataPointsResponse{}

	transferSpeedFilter := createFilter(query, transferSpeedFilterGenerator)
	transferSpeed, err := queryTransferSpeed(s, ctx, startTime, endTime, intervalInSec, transferSpeedFilter)
	if err != nil {
		return nil, err
	}
	response.TransferRates = transferSpeed

	fileRateFilter := createFilter(query, transferRateFilterGenerator)
	fileRates, err := queryFileRate(s, ctx, startTime, endTime, intervalInSec, fileRateFilter)
	if err != nil {
		return nil, err
	}
	response.FileRates = fileRates

	taskTimeFilter := createFilter(query, taskTimeFilterGenerator)
	taskTime, err := queryTaskTime(s, ctx, startTime, endTime, intervalInSec, taskTimeFilter)
	if err != nil {
		return nil, err
	}
	response.TaskTimes = taskTime

	memoryUsageFilter := createFilter(query, memoryUsageFilterGenerator)
	memoryUsage, err := queryMemoryUsage(s, ctx, startTime, endTime, intervalInSec, memoryUsageFilter)
	if err != nil {
		return nil, err
	}
	response.MemoryUsages = memoryUsage

	response.StartTimestampInSec = query.FromTimestamp // TODO: Use first timestamp from query result
	response.TimeIntervalInSec = uint32(intervalInSec)

	return &response, nil
}

func queryTransferSpeed(s *QueryServer, ctx context.Context, startTime string, endTime string, intervalInSec int, filter string) ([]*pb.TotalTransferRateDataPoint, error) {
	var arrayResult []*pb.TotalTransferRateDataPoint

	result, err := s.dbQueryApi.Query(ctx, "from(bucket: \""+s.settings.InfluxDb2Bucket+"\")"+
		" |> range(start: "+startTime+", stop: "+endTime+")"+
		" |> filter(fn: (r) => r._measurement == \""+dbMeasurementFileInput+"\" or r._measurement == \""+dbMeasurementBrokerFileRequests+"\" or r._measurement == \""+dbMeasurementRdsFileRequests+"\" or r._measurement == \""+dbMeasurementFtsTransfers+"\")"+
		" |> filter(fn: (r) => r._field == \"totalInputFileSize\" or r._field == \"totalRequestedFileSize\" or r._field == \"totalRdsOutputFileSize\" or r._field == \"totalFtsTransferredFileSize\")"+
		filter +
		" |> group(columns: [\"_field\"])"+
		" |> aggregateWindow(every: "+strconv.Itoa(intervalInSec)+"s, fn: sum, createEmpty: true)"+
		" |> pivot(rowKey:[\"_time\"], columnKey: [\"_field\"], valueColumn: \"_value\")",
	)

	if err != nil {
		return nil, err
	}

	for result.Next() {
		recvBytes := uint64(0)
		sendBytes := uint64(0)
		sendRdsBytes := uint64(0)
		sendFtsBytes := uint64(0)

		if result.Record().Values()["totalInputFileSize"] != nil {
			recvBytes = result.Record().Values()["totalInputFileSize"].(uint64)
		}
		if result.Record().Values()["totalRequestedFileSize"] != nil {
			sendBytes = result.Record().Values()["totalRequestedFileSize"].(uint64)
		}
		if result.Record().Values()["totalRdsOutputFileSize"] != nil {
			sendRdsBytes = result.Record().Values()["totalRdsOutputFileSize"].(uint64)
		}
		if result.Record().Values()["totalFtsTransferredFileSize"] != nil {
			sendFtsBytes = result.Record().Values()["totalFtsTransferredFileSize"].(uint64)
		}

		arrayResult = append(arrayResult, &pb.TotalTransferRateDataPoint{
			TotalBytesPerSecRecv:    recvBytes / uint64(intervalInSec),
			TotalBytesPerSecSend:    sendBytes / uint64(intervalInSec),
			TotalBytesPerSecRdsSend: sendRdsBytes / uint64(intervalInSec),
			TotalBytesPerSecFtsSend: sendFtsBytes / uint64(intervalInSec),
		})
	}

	if result.Err() != nil {
		return nil, result.Err()
	}

	return arrayResult, nil
}

func queryFileRate(s *QueryServer, ctx context.Context, startTime string, endTime string, intervalInSec int, filter string) ([]*pb.TotalFileRateDataPoint, error) {
	var arrayResult []*pb.TotalFileRateDataPoint

	rdsResponses, err := s.dbQueryApi.Query(ctx, "from(bucket: \""+s.settings.InfluxDb2Bucket+"\")"+
		" |> range(start: "+startTime+", stop: "+endTime+")"+
		" |> filter(fn: (r) => r._measurement == \""+dbMeasurementRdsFileRequests+"\" or r._measurement == \""+dbMeasurementBrokerFileRequests+"\" or r._measurement == \""+dbMeasurementFtsTransfers+"\")"+
		" |> filter(fn: (r) => r._field == \"totalRdsHits\" or r._field == \"totalRdsMisses\" or r._field == \"requestedFileCount\" or r._field == \"ftsFileCount\")"+
		filter +
		" |> group(columns: [\"_field\"])"+
		" |> aggregateWindow(every: "+strconv.Itoa(intervalInSec)+"s, fn: sum, createEmpty: true)"+
		" |> pivot(rowKey:[\"_time\"], columnKey: [\"_field\"], valueColumn: \"_value\")",
	)
	if err != nil {
		return nil, err
	}

	for rdsResponses.Next() {
		totalRequests := uint32(0)
		hit := uint32(0)
		miss := uint32(0)
		fromDisk := uint32(0)

		if rdsResponses.Record().Values()["requestedFileCount"] != nil {
			totalRequests = uint32(rdsResponses.Record().Values()["requestedFileCount"].(uint64))
		}
		if rdsResponses.Record().Values()["totalRdsHits"] != nil {
			hit = uint32(rdsResponses.Record().Values()["totalRdsHits"].(uint64))
		}
		if rdsResponses.Record().Values()["totalRdsMisses"] != nil {
			miss = uint32(rdsResponses.Record().Values()["totalRdsMisses"].(uint64))
		}
		if rdsResponses.Record().Values()["ftsFileCount"] != nil {
			fromDisk = uint32(rdsResponses.Record().Values()["ftsFileCount"].(uint64))
		}

		arrayResult = append(arrayResult, &pb.TotalFileRateDataPoint{
			TotalRequests: totalRequests,
			CacheMisses:   miss,
			FromCache:     hit,
			FromDisk:      fromDisk,
		})
	}

	if rdsResponses.Err() != nil {
		return nil, rdsResponses.Err()
	}

	return arrayResult, nil
}

func queryTaskTime(s *QueryServer, ctx context.Context, startTime string, endTime string, intervalInSec int, filter string) ([]*pb.TaskTimeDataPoint, error) {
	var arrayResult []*pb.TaskTimeDataPoint

	result, err := s.dbQueryApi.Query(ctx, "from(bucket: \""+s.settings.InfluxDb2Bucket+"\")"+
		" |> range(start: "+startTime+", stop: "+endTime+")"+
		" |> filter(fn: (r) => r._measurement == \""+dbMeasurementFileInput+"\" or r._measurement == \""+dbMeasurementRdsFileRequests+"\")"+
		" |> filter(fn: (r) => r._field == \"avgTransferReceiveTimeUs\" or r._field == \"avgWriteIoTimeUs\" or r._field == \"avgDbTimeUs\" or r._field == \"avgRdsOutputTransferTimeUs\")"+
		filter +
		" |> group(columns: [\"_field\"])"+
		" |> aggregateWindow(every: "+strconv.Itoa(intervalInSec)+"s, fn: mean, createEmpty: true)"+
		" |> pivot(rowKey:[\"_time\"], columnKey: [\"_field\"], valueColumn: \"_value\")",
	)
	if err != nil {
		return nil, err
	}

	for result.Next() {
		receiveIoTimeUs := float64(0)
		writeToDiskTimeUs := float64(0)
		writeToDatabaseTimeUs := float64(0)
		rdsOutputTransferTimeUs := float64(0)

		if result.Record().Values()["avgTransferReceiveTimeUs"] != nil {
			receiveIoTimeUs = result.Record().Values()["avgTransferReceiveTimeUs"].(float64)
		}
		if result.Record().Values()["avgWriteIoTimeUs"] != nil {
			writeToDiskTimeUs = result.Record().Values()["avgWriteIoTimeUs"].(float64)
		}
		if result.Record().Values()["avgDbTimeUs"] != nil {
			writeToDatabaseTimeUs = result.Record().Values()["avgDbTimeUs"].(float64)
		}
		if result.Record().Values()["avgRdsOutputTransferTimeUs"] != nil {
			rdsOutputTransferTimeUs = result.Record().Values()["avgRdsOutputTransferTimeUs"].(float64)
		}

		arrayResult = append(arrayResult, &pb.TaskTimeDataPoint{
			ReceiveIoTimeUs:         uint32(receiveIoTimeUs),
			WriteToDiskTimeUs:       uint32(writeToDiskTimeUs),
			WriteToDatabaseTimeUs:   uint32(writeToDatabaseTimeUs),
			RdsSendToConsumerTimeUs: uint32(rdsOutputTransferTimeUs),
		})
	}

	if result.Err() != nil {
		return nil, result.Err()
	}

	return arrayResult, nil
}

func queryMemoryUsage(s *QueryServer, ctx context.Context, startTime string, endTime string, intervalInSec int, filter string) ([]*pb.RdsMemoryUsageDataPoint, error) {
	var arrayResult []*pb.RdsMemoryUsageDataPoint

	result, err := s.dbQueryApi.Query(ctx, "from(bucket: \""+s.settings.InfluxDb2Bucket+"\")"+
		" |> range(start: "+startTime+", stop: "+endTime+")"+
		" |> filter(fn: (r) => r._measurement == \""+dbMeasurementRdsCacheMemoryUsage+"\")"+
		" |> filter(fn: (r) => r._field == \"rdsCacheUsedBytes\")"+
		filter +
		" |> group(columns: [\"_field\"])"+
		" |> aggregateWindow(every: "+strconv.Itoa(intervalInSec)+"s, fn: sum, createEmpty: true)"+
		" |> pivot(rowKey:[\"_time\"], columnKey: [\"_field\"], valueColumn: \"_value\")",
	)
	if err != nil {
		return nil, err
	}

	for result.Next() {
		usedBytes := uint64(0)

		if result.Record().Values()["rdsCacheUsedBytes"] != nil {
			usedBytes = result.Record().Values()["rdsCacheUsedBytes"].(uint64)
		}

		arrayResult = append(arrayResult, &pb.RdsMemoryUsageDataPoint{
			TotalUsedMemory: usedBytes,
		})
	}

	if result.Err() != nil {
		return nil, result.Err()
	}

	return arrayResult, nil
}

func (s *QueryServer) GetTopology(ctx context.Context, query *pb.ToplogyQuery) (*pb.TopologyResponse, error) {
	if doesStringContainsDangerousElements(query.BeamtimeFilter) {
		return nil, errors.New("some input characters are not allowed")
	}

	startTime := strconv.FormatUint(query.FromTimestamp, 10)
	endTime := strconv.FormatUint(query.ToTimestamp, 10)

	result, err := s.dbQueryApi.Query(ctx, "from(bucket: \""+s.settings.InfluxDb2Bucket+"\")"+
		" |> range(start: "+startTime+", stop: "+endTime+")"+
		" |> filter(fn: (r) => r._measurement == \""+dbMeasurementFileInput+"\" or r._measurement == \""+dbMeasurementBrokerFileRequests+"\")" +
		" |> filter(fn: (r) => " + createFilterElement("beamtime", query.BeamtimeFilter) + ")" +
		" |> keep(columns: [\"receiverName\", \"brokerName\", \"pipelineStepId\", \"source\", \"producerInstanceId\", \"consumerInstanceId\"])" +
		" |> group()",
	)

	if err != nil {
		fmt.Printf("Error: %v\n", err)
		return nil, err
	}

	type PipelineStep struct { // these maps here are replacements for HashSets
		producesSourceId map[string] /*produces sourceId*/ bool
		consumesSourceId map[string] /*consumers sourceId*/ bool

		producerInstances map[string] /*produces instanceId*/ bool
		consumerInstances map[string] /*consumers instanceId*/ bool

		involvedReceivers map[string] /*receiver id*/ bool
	}

	pipelineSteps := map[string] /*pipelineStepId*/ PipelineStep{}

	getOrCreateStep := func(stepId string) PipelineStep {
		if _, containsKey := pipelineSteps[stepId]; !containsKey {
			pipelineSteps[stepId] = PipelineStep{
				producesSourceId:  map[string] /*produces sourceId*/ bool{},
				consumesSourceId:  map[string] /*consumers sourceId*/ bool{},
				producerInstances: map[string] /*produces instanceId*/ bool{},
				consumerInstances: map[string] /*consumers instanceId*/ bool{},
				involvedReceivers: map[string] /*receiver id*/ bool{},
			}
		}
		return pipelineSteps[stepId]
	}

	for result.Next() {
		if result.Record().Values()["receiverName"] != nil { // data is coming from receiver => means it must be a producer
			stepId := result.Record().Values()["pipelineStepId"].(string)
			source := result.Record().Values()["source"].(string)
			producerInstanceId := result.Record().Values()["producerInstanceId"].(string)
			var step = getOrCreateStep(stepId)
			step.producesSourceId[source] = true
			step.producerInstances[producerInstanceId] = true
			step.involvedReceivers[result.Record().Values()["receiverName"].(string)] = true
		} else if result.Record().Values()["brokerName"] != nil { // data is coming from broker => means it must be a consumer
			stepId := result.Record().Values()["pipelineStepId"].(string)
			source := result.Record().Values()["source"].(string)
			consumerInstanceId := result.Record().Values()["consumerInstanceId"].(string)
			var step = getOrCreateStep(stepId)
			step.consumesSourceId[source] = true
			step.consumerInstances[consumerInstanceId] = true
		} else {
			log.Debug("Got an entry without receiverName or brokerName")
		}
	}

	type PipelineLevelStepInfo struct {
		stepId string
	}

	type PipelineEdgeInfo struct {
		fromStepId        string
		toStepId          string
		sourceId          string
		involvedReceivers map[string] /*receiver id*/ bool
	}

	// Simple algorithm
	// Goes through each remainingStepIds and checks if all consumesSourceIds of this step is available
	//    If all consuming sources are available
	//      add it to the current level
	//      add all connecting edges
	//      and producing sources to availableSourcesInNextLevel
	//    If not go to next step, which might produces this
	// When an iteration of remainingStepIds is done:
	//    Add all new producing sources to availableSources
	//    Go to next level
	remainingStepIds := map[string] /*pipelineStepId*/ bool{}
	availableSources := map[string] /*sourceId*/ [] /* available from pipelineStepId */ string{}

	for stepId, _ := range pipelineSteps {
		remainingStepIds[stepId] = true
	}

	var levels [] /*levelDepth*/ [] /*verticalIdx*/ PipelineLevelStepInfo
	var edges []PipelineEdgeInfo
	var currentLevel = -1

	for len(remainingStepIds) > 0 {
		currentLevel++
		levels = append(levels, []PipelineLevelStepInfo{})

		var foundAtLeastOne = false
		availableSourcesInNextLevel := map[string] /*sourceId*/ [] /* available from pipelineStepId */ string{}
		for stepId := range remainingStepIds {
			var allConsumingSourcesAvailable = true
			for requiredSourceId := range pipelineSteps[stepId].consumesSourceId {
				if _, sourceIsAvailable := availableSources[requiredSourceId]; !sourceIsAvailable { // Oh source is unavailable
					allConsumingSourcesAvailable = false
				} else {
					// Verify that no other producer can create this source.
					// Maybe we have to wait one or two steps until all producers are available
					for stepId2 := range remainingStepIds {
						if _, thereIsAnotherProducer := pipelineSteps[stepId2].producesSourceId[requiredSourceId]; thereIsAnotherProducer {
							if stepId == stepId2 {
								continue // The pipeline has self reference?! allow it
							}
							allConsumingSourcesAvailable = false
						}
					}
				}
				if !allConsumingSourcesAvailable {
					break // We don't even need to try the other options
				}
			}

			if allConsumingSourcesAvailable {
				foundAtLeastOne = true
				stepInfo := pipelineSteps[stepId]

				// Add edge connection
				for consumingSourceId := range stepInfo.consumesSourceId {
					if stepInfo.producesSourceId[consumingSourceId] { // Add self reference edge
						edges = append(edges, PipelineEdgeInfo{
							fromStepId:        stepId,
							toStepId:          stepId,
							sourceId:          consumingSourceId,
							involvedReceivers: stepInfo.involvedReceivers,
						})
					}
					for _, sourceIdAvailableFromStepId := range availableSources[consumingSourceId] { // Add all the others edges
						edges = append(edges, PipelineEdgeInfo{
							fromStepId:        sourceIdAvailableFromStepId,
							toStepId:          stepId,
							sourceId:          consumingSourceId,
							involvedReceivers: stepInfo.involvedReceivers,
						})
					}
				}

				// prepare sources for next level
				for sourceId := range stepInfo.producesSourceId {
					if _, sourceIsAvailable := availableSourcesInNextLevel[sourceId]; !sourceIsAvailable {
						availableSourcesInNextLevel[sourceId] = nil
					}

					availableSourcesInNextLevel[sourceId] = append(availableSourcesInNextLevel[sourceId], stepId)
				}

				levels[currentLevel] = append(levels[currentLevel], PipelineLevelStepInfo{
					stepId: stepId,
				})

				delete(remainingStepIds, stepId)
			}
		}

		if !foundAtLeastOne {
			// probably only requests of files came, but no receiver registered a producer
			log.Error("infinite loop while building topology tree; Still has pipeline steps but found no way how to connect them")
			return nil, errors.New("infinite loop while building topology tree; Still has pipeline steps but found no way how to connect them")
		}

		for sourceId, element := range availableSourcesInNextLevel {
			if _, sourceIsAvailable := availableSources[sourceId]; !sourceIsAvailable {
				availableSources[sourceId] = nil
			}

			for _, newSource := range element {
				availableSources[sourceId] = append(availableSources[sourceId], newSource)
			}
		}
	}

	var response pb.TopologyResponse

	for levelIndex, level := range levels {
		for _, node := range level {
			var producers []string = nil
			var consumers []string = nil

			for producerInstanceId := range pipelineSteps[node.stepId].producerInstances {
				producers = append(producers, producerInstanceId)
			}
			for consumerInstanceId := range pipelineSteps[node.stepId].consumerInstances {
				consumers = append(consumers, consumerInstanceId)
			}
			response.Nodes = append(response.Nodes, &pb.TopologyResponseNode{
				NodeId:            node.stepId,
				Level:             uint32(levelIndex),
				ProducerInstances: producers,
				ConsumerInstances: consumers,
			})
		}
	}

	for _, edge := range edges {
		newEdge := pb.TopologyResponseEdge{
			FromNodeId:        edge.fromStepId,
			ToNodeId:          edge.toStepId,
			SourceName:        edge.sourceId,
			InvolvedReceivers: nil,
		}

		for receiverId := range edge.involvedReceivers {
			newEdge.InvolvedReceivers = append(newEdge.InvolvedReceivers, receiverId)
		}

		response.Edges = append(response.Edges, &newEdge)
	}

	return &response, nil
}
