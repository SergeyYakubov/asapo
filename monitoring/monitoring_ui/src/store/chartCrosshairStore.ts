import { reactive } from "vue";

export interface ChartCrosshairStoreState {
    xValue: number | null;
}

class ChartCrosshairStore {
    private internalState: ChartCrosshairStoreState = reactive({
        xValue: null,
    });

    public get state(): Readonly<ChartCrosshairStoreState> {
        return this.internalState;
    }

    public showAndUpdate(xValue: number) {
        this.internalState.xValue = xValue;
    }

    public hide() {
        this.internalState.xValue = null;
    }
}

export const chartCrosshairStore = new ChartCrosshairStore();
