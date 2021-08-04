export interface PipelineNode {
    id: string;
    level: number;
    consumerCount: number;
    producerCount: number;

    consumers: string[];
    producers: string[];    
}

export interface PipelineEdge {
    id: string;

    fromId: string;
    toId: string;
    sourceName: string;

    receivers: string[];

    //filesPerSec: number;
    //miBPerSec: number;
}

export interface Pipeline { 
    nodes: PipelineNode[];
    edges: PipelineEdge[];
}

