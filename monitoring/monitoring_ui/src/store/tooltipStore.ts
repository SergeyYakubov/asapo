import { reactive } from "vue";

export interface TooltipTableInfo {
    name: string;
    value: string;
}

export interface TooltipStoreState {
    visible: boolean;

    posX: number;
    posY: number;

    titleLine: string;
    lines: TooltipTableInfo[];
}

class TooltipStore {
    private internalState: TooltipStoreState = reactive({
        visible: false,

        posX: 0,
        posY: 0,

        titleLine: '',
        lines: [],
    });

    public get state(): Readonly<TooltipStoreState> {
        return this.internalState;
    }

    public showAndUpdate(posX: number, posY: number, titleLine: string, lines: TooltipTableInfo[]) {
        this.internalState.visible = true;
        this.internalState.posX = posX;
        this.internalState.posY = posY;
        this.internalState.titleLine = titleLine;
        this.internalState.lines = lines;
    }

    public hide() {
        this.internalState.visible = false;
    }
}

export const tooltipStore = new TooltipStore();
