import { reactive } from "vue";

export interface ErrorStoreState {
    connectionMetadataErrorText: string | null;
    connectionMetricsErrorText: string | null;
    connectionToplogyErrorText: string | null;
    filterErrorText: string | null;
    timeErrorText: string | null;
}

class ErrorStore {
    private internalState: ErrorStoreState = reactive({
        connectionMetadataErrorText: null,
        connectionMetricsErrorText: null,
        connectionToplogyErrorText: null,
        filterErrorText: null,
        timeErrorText: null,
    });

    public get state(): Readonly<ErrorStoreState> {
        return this.internalState;
    }

    public setMetadataConnectionErrorText(text: string): void {
        this.internalState.connectionMetadataErrorText = text;
    }
    public clearMetadataConnectionError(): void {
        this.internalState.connectionMetadataErrorText = null;
    }

    public setMetricsConnectionErrorText(text: string): void {
        this.internalState.connectionMetricsErrorText = text;
    }
    public clearMetricsConnectionError(): void {
        this.internalState.connectionMetricsErrorText = null;
    }

    public setToplogyConnectionErrorText(text: string): void {
        this.internalState.connectionToplogyErrorText = text;
    }
    public clearToplogyConnectionError(): void {
        this.internalState.connectionToplogyErrorText = null;
    }

    public setFilterErrorText(text: string): void {
        this.internalState.filterErrorText = text;
    }
    public clearFilterErrorText(): void {
        this.internalState.filterErrorText = null;
    }

    public setTimeErrorText(text: string): void {
        this.internalState.timeErrorText = text;
    }
    public clearTimeErrorText(): void {
        this.internalState.timeErrorText = null;
    }

    public get errorText(): string | null {
        if (this.state.filterErrorText) {
            return `Filter Error: ${this.state.filterErrorText}`;
        }
        if (this.state.timeErrorText) {
            return `Time: ${this.state.timeErrorText}`;
        }

        if (this.state.connectionMetadataErrorText) {
            return `Metadata: ${this.state.connectionMetadataErrorText}`;
        }
        if (this.state.connectionMetricsErrorText) {
            return `Metrics: ${this.state.connectionMetricsErrorText}`;
        }
        if (this.state.connectionToplogyErrorText) {
            return `Toplogy: ${this.state.connectionToplogyErrorText}`;
        }
        
        // possible other errors here

        return null;
    }
}

export const errorStore = new ErrorStore();
