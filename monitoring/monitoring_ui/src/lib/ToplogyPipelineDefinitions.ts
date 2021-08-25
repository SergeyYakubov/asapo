export interface PipelineNode {
    id: string;
    level: number;

    consumers: string[];
    producers: string[];    
}

export interface PipelineEdge {
    id: string;

    fromId: string;
    toId: string;
    sourceName: string;

    receivers: string[];
}

export interface Pipeline { 
    nodes: PipelineNode[];
    edges: PipelineEdge[];
}

