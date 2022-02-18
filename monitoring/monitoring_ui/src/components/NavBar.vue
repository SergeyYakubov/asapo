<template>
    <nav class="flex justify-between p-2 bg-gray-300 border-b border-gray-600">
        <!-- left -->
        <div class="flex-1 flex items-center justify-between">
            <div>
                <h1 title="TODO Version and build commit/data">ASAPO Monitoring</h1>
                <p class="text-xs">Cluster: <span class="font-mono">{{clusterName}}</span></p>
            </div>
            <div class="ml-5 flex-1 text-red-600 font-bold" v-if="errorMessage">
                <p>{{errorMessage}}</p>
            </div>
        </div>

        <!-- center -->
        <div class="flex flex-col items-center justify-center">
            <h1 class="font-bold text-xl">Dashboard</h1>
            <p>For beamtime {{currentBeamtime}}</p>
        </div>

        <!-- right -->
        <div class="flex-1 flex items-center justify-end">
            <div class="flex items-center justify-end mx-2 bg-gray-300">
                <FilterSelector />
            </div>
            <div class="flex items-center justify-end">
                <TimeSelector />
            </div>
        </div>
    </nav>
</template>

<script lang="ts">
import { Options, Vue } from "vue-class-component";
import TimeSelector from '../components/TimeSelector.vue';
import FilterSelector from '../components/FilterSelector.vue';
import { errorStore } from "../store/errorStore";
import { connection } from "../store/connectionStore";
import { selectionFilterStore } from "../store/selectionFilterStore";

@Options({
    components: {
        TimeSelector,
        FilterSelector,
    }
})
export default class NavBar extends Vue {
    private get currentBeamtime(): string | null {
        if (selectionFilterStore.state.beamtime) {
            return selectionFilterStore.state.beamtime;
        }
        return "<Must be set>";
    }

    private get errorMessage(): string | null {
        return errorStore.errorText;
    }

    private get clusterName(): string {
        return connection.state.clusterName;
    }
}
</script>

<style>

</style>
