import { reactive } from "vue";
import { TopologyResponse } from "../generated_proto/AsapoMonitoringQueryService_pb";
import { FixedTimeRange } from "../lib/TimeRange";
import { Pipeline, PipelineNode, PipelineEdge } from "../lib/ToplogyPipelineDefinitions";
import { connection } from "./connectionStore";
import { timeStore } from "./timeStore";

export interface ToplogyStoreState {
    lastUpdateRange: FixedTimeRange,
    pipeline: Pipeline;
    nodeMap: NodeMap;
    edgeMap: EdgeMap;
}

type NodeMap = {[nodeId: string]: PipelineNode};
type EdgeMap = {[edgeId: string]: PipelineEdge};

class ToplogyStore {
    private internalState: ToplogyStoreState = reactive({
        lastUpdateRange: {
            fromUnixSec: 0,
            toUnixSec: 0
        },
        pipeline: {
            nodes: [],
            edges: [],
        },
        nodeMap: {},
        edgeMap: {},
    });

    public get state(): Readonly<ToplogyStoreState> {
        return this.internalState;
    }

    public parseServerResponse(toplogyData: TopologyResponse): void {
        const newNodes: PipelineNode[] = [];
        const newNodeMap: NodeMap = {};
        for(const node of toplogyData.getNodesList()) {
            const newNode = {
                id: node.getNodeid(),
                level: node.getLevel(),

                producers: node.getProducerinstancesList(),
                producerCount: node.getProducerinstancesList().length,

                consumers: node.getConsumerinstancesList(),
                consumerCount: node.getConsumerinstancesList().length,
            };
            newNodes.push(newNode);
            newNodeMap[newNode.id] = newNode;
        }

        const newEdges: PipelineEdge[] = [];
        const newedgeMap: EdgeMap = {};
        for(const edge of toplogyData.getEdgesList()) {
            const newEdge = {
                id: '',
                fromId: edge.getFromnodeid(),
                toId: edge.getTonodeid(),

                receivers: edge.getInvolvedreceiversList(),
                sourceName: edge.getSourcename(),
            };

            newEdge.id = `${newEdge.fromId}~>${newEdge.sourceName}~>${newEdge.toId}`;
            newEdges.push(newEdge);
            newedgeMap[newEdge.id] = newEdge;
        }

        this.internalState.nodeMap = newNodeMap;
        this.internalState.edgeMap = newedgeMap;
        this.internalState.pipeline = {
            nodes: newNodes,
            edges: newEdges,
        };

        this.internalState.lastUpdateRange.fromUnixSec = timeStore.state.lastFixedTimeRange.fromUnixSec;
        this.internalState.lastUpdateRange.toUnixSec = timeStore.state.lastFixedTimeRange.toUnixSec;
    }

    public couldBeOutdated(): boolean {
        const maxPipelineToplogyOldnessInSec = 60;
        
        return  Math.abs(this.internalState.lastUpdateRange.fromUnixSec - timeStore.state.lastFixedTimeRange.fromUnixSec) > maxPipelineToplogyOldnessInSec ||
                Math.abs(this.internalState.lastUpdateRange.toUnixSec - timeStore.state.lastFixedTimeRange.toUnixSec) > maxPipelineToplogyOldnessInSec
    }

    public getAvailableSources(): string[] {
        const set = new Set<string>();

        for (const edge of this.state.pipeline.edges) {
            set.add(edge.sourceName);
        }

        return [...set];
    }

    public getAvailableStreams(): string[] {
        return [];
    }
}

export const toplogyStore = new ToplogyStore();
