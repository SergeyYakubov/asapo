<template>
    <div
      class="bg-gray-300 border border-gray-700 p-1 w-max"
      :class="state.visible ? 'absolute' : 'hidden'"
      :style="{left: `${posX}px`, top: `${posY}px`}">
        <p class="font-bold">{{titleLine}}</p>
        <table>
            <tr v-for="line in lines" :key="line.name">
                <td>{{line.name}}</td>
                <td>{{line.value}}</td>
            </tr>
        </table>
    </div>
</template>

<script lang="ts">
import { Vue } from "vue-class-component";
import { tooltipStore, TooltipStoreState, TooltipTableInfo } from "../store/tooltipStore";

export default class Tooltip extends Vue {
    
    public static readonly offset = 10;//px4
    public static readonly overflowPrevention = 30;//px

    public get state(): TooltipStoreState {
        return tooltipStore.state;
    }
    public get titleLine(): string {
        return this.state.titleLine;
    }
    public get lines(): TooltipTableInfo[] {
        return this.state.lines;
    }

    public get posX(): number {
        const intendedPosX = this.state.posX + Tooltip.offset;

        // overflow check
        if (this.$el && (intendedPosX + this.$el.clientWidth + Tooltip.overflowPrevention) > window.innerWidth) {
            return this.state.posX - Tooltip.offset - this.$el.clientWidth;
        }

        return intendedPosX;
    }

    public get posY(): number {
        return this.state.posY + Tooltip.offset;
    }
}

</script>
1
<style>

</style>