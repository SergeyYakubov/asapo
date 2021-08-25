<template>
    <div class="w-full h-full">
        <div class="flex flex-row items-center">
            <p class="mr-2 font-bold">Topology</p>
            <p class="mr-2">Time: {{updateDateText}}</p>
            <button :disabled="isFetchingToplogyData" @click="reloadToplogy()">
                <img src="../assets/icons/single_refresh.svg" class="w-8 h-8" :class="isFetchingToplogyData ? 'animate-spin' : ''" />
            </button>
        </div>
        <div class="w-full h-full">
            <div class="w-full h-full bg-white" :ref="vueSetVisBody">
            </div>
        </div>
    </div>
</template>

<script lang="ts">
import { Options, Vue } from "vue-class-component";
import { connection } from "../store/connectionStore";
import { Pipeline } from "../lib/ToplogyPipelineDefinitions";
import * as vis from 'vis-network';
import { selectionFilterStore } from "../store/selectionFilterStore";
import { toplogyStore } from "../store/toplogyStore";
import moment from 'moment';
import { FixedTimeRange } from "../lib/TimeRange";

@Options({
    watch: {
        filterState: {
            handler() {
                this.filterChanged();
            },
            deep: true,
        },
        toplogyStoreLastUpdate: {
            handler() {
                this.topologyChanged();
            },
            deep: true,
        },
    }
})
export default class TopologyGraph extends Vue {    
    private get filterState(): typeof selectionFilterStore.state {
        return selectionFilterStore.state
    }
    private get toplogyStoreLastUpdate(): FixedTimeRange {
        return toplogyStore.state.lastUpdateRange;
    }

    private get updateDateText() {
        return moment.unix(this.toplogyStoreLastUpdate.fromUnixSec).format('HH:mm:ss') + ' - ' + moment.unix(this.toplogyStoreLastUpdate.toUnixSec).format('HH:mm:ss')
    }

    private visBody!: HTMLDivElement;

    public mounted() {
        const visOptions: vis.Options = {
            edges: {
                smooth: {
                    enabled: true,
                    type: 'cubicBezier',
                    forceDirection: 'horizontal',
                    roundness: 0.4,
                },
                color: '#000000',
                arrows: {
                    to: {
                        enabled: true,
                        scaleFactor: 0.6,
                    },
                },
                font: {
                    size: 10,
                }
            },
            nodes: {
                shape: 'custom',
                color: '#d1d5db',
                ctxRenderer: ({ ctx, x, y, state: { selected, hover }, style, label, id}: 
                {ctx: CanvasRenderingContext2D, x: number, y: number, state: { selected: boolean, hover: boolean}, style: any, label?: string, id: string}) => {
                    const width = 70;
                    const height = 42;
                    const labelMargin = 10;
                    const innerTextMargin = 5;
                    
                    return {
                        drawNode() {
                            ctx.save();

                            ctx.beginPath();
                            ctx.fillStyle = style.color;
                            ctx.strokeStyle = style.borderColor;
                            ctx.lineWidth = style.borderWidth;
                            ctx.rect(x - (width/2), y - (height/2), width, height);
                            ctx.closePath();

                            ctx.fill();
                            ctx.stroke();

                            ctx.restore();
                        },
                        drawExternalLabel() {
                            ctx.save();

                            ctx.fillStyle = '#000000';
                            ctx.textBaseline = 'middle';

                            if (label) {
                                const fontPx = selected ? 14 : 10;
                                ctx.font =  `${fontPx}px Arial`;
                                ctx.textAlign = 'center';
                                ctx.fillText(label, x, y - (height/2) - labelMargin);
                            }

                            const nodeInfo = toplogyStore.state.nodeMap[id];
                            if (nodeInfo) {
                                ctx.font = 20 + "px monospace";
                                ctx.textAlign = 'left';
                                ctx.fillText(`${nodeInfo.consumers.length}`, x - (width/2) + innerTextMargin, y + 6);
                                ctx.font = 5 + "px monospace";
                                ctx.fillText('consumers', x - (width/2) + innerTextMargin, y - 10);

                                ctx.font = 20 + "px monospace";
                                ctx.textAlign = 'right';
                                ctx.fillText(`${nodeInfo.producers.length}`, x + (width/2) - innerTextMargin, y + 6);
                                ctx.font = 5 + "px monospace";
                                ctx.fillText('producers', x + (width/2) - innerTextMargin, y - 10);
                            }

                            ctx.restore();
                        },
                        nodeDimensions: {
                            width,
                            height,
                        },
                    };
                }
            } as any,
            layout: {
                hierarchical: {
                    direction: 'LR',
                },
            },
            physics: false,
        };

        this.network = new vis.Network(this.visBody, {}, visOptions);
        this.network.on('click', this.onNetworkClick);

        this.topologyChanged();
    }

    private static convertToVisData(pipeline: Pipeline): vis.Data {
        const visNodes: vis.Node[] = [];
        const visEdges: vis.Edge[] = [];

        for (const node of pipeline.nodes) {
            visNodes.push({
                id: node.id,
                label: node.id,
                level: node.level,
            });
        }

        for (const edge of pipeline.edges) {
            visEdges.push({
                id: edge.id,
                from: edge.fromId,
                to: edge.toId,
                label: edge.sourceName, // TODO: // + '\n' + edge.filesPerSec + 'f/s\n' + edge.miBPerSec + 'MiB/s',
            });
        }

        return {
            nodes: visNodes,
            edges: visEdges,
        };
    }

    private filterChanged(): void {
        if (!selectionFilterStore.hasClearableFilter) {
            this.network.setSelection({
                nodes: [],
                edges: [],
            }, {unselectAll: true, highlightEdges: false});
            return;
        }

        let nodes = Object.values(toplogyStore.state.nodeMap);
        if (selectionFilterStore.state.fromPipelineStepId && selectionFilterStore.state.toPipelineStepId) {
            nodes = nodes.filter(node => node.id === selectionFilterStore.state.fromPipelineStepId || node.id === selectionFilterStore.state.toPipelineStepId);
        }

        let edges = Object.values(toplogyStore.state.edgeMap)

        if (selectionFilterStore.state.source) {
            edges = edges.filter(edge => edge.sourceName === selectionFilterStore.state.source);
        }
        if (selectionFilterStore.state.receiverId) {
            edges = edges.filter(edge => edge.receivers.includes(selectionFilterStore.state.receiverId!));
        }
        if (selectionFilterStore.state.fromPipelineStepId && selectionFilterStore.state.toPipelineStepId) {
            edges = edges.filter(edge => edge.fromId === selectionFilterStore.state.fromPipelineStepId && edge.toId === selectionFilterStore.state.toPipelineStepId);
        }


        this.network.setSelection({
            nodes: nodes.map(node => node.id),
            edges: edges.map(edge => edge.id),
        }, {unselectAll: true, highlightEdges: false})
    }

    private topologyChanged(): void {
        this.updateData(toplogyStore.state.pipeline);
    }

    private onNetworkClick(props: any): void {
        this.network.unselectAll();
        if (props.nodes.length === 1) {
            // const node = (this.network as any).body.nodes[props.nodes[0]] as vis.Node;
            selectionFilterStore.setFilterPipelineStep(props.nodes[0]);
        } else if (props.edges.length === 1) {
            //const edge = (this.network as any).body.edges[props.edges[0]] as vis.Edge;
            const edge = toplogyStore.state.edgeMap[props.edges[0]];
            selectionFilterStore.setFilterSourceWithPipeline(edge.sourceName, edge.fromId, edge.toId);
        } else {
            selectionFilterStore.clearFilter();
        }
        this.filterChanged();
    }

    private network!: vis.Network;

    public updateData(pipeline: Pipeline) {
        const data = TopologyGraph.convertToVisData(pipeline);

        this.network.setData(data);
        this.network.fit({
            minZoomLevel: 1.6,
            maxZoomLevel: 1.6,
        });
    }

    public beforeUnmount() {
        this.network.destroy();
    }

    private vueSetVisBody(visBody: HTMLDivElement): void {
        this.visBody = visBody;
    }

    private get isFetchingToplogyData(): boolean {
        return connection.state.isFetchingToplogyData;
    }

    private reloadToplogy(): void {
        return connection.triggerToplogyRefresh();
    }
}
</script>

<style lang="scss">
.customclass {
    canvas {
        position: absolute !important;
    }
}
</style>