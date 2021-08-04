<template>
    <div>
        <div class="w-full h-full bg-gray-100 border border-gray-600 m-1 p-1">
            <div class="flex place-content-between">
                <p class="font-bold">{{label}}</p>
            </div>
            <ul class="overflow-y-scroll font-mono mt-1 bg-gray-100" style="width: 100%; height: 190px" :class="onElementClicked ? 'clickableList' : ''">
                <template v-if="onElementClicked">
                    <li v-for="element in elements" :key="element" @click="onElementClicked(element)" :class="{'selected': isElementedSelectedFallback(element)}">{{element}}</li>
                </template>
                <template v-else>
                    <li v-for="element in elements" :key="element">{{element}}</li>
                </template>
            </ul>
        </div>
    </div>
</template>

<script lang="ts">
import { Options, Vue } from "vue-class-component";

@Options({
    props: {
        label: String,
        elements: Object, // string[]

        onElementClicked: Function,  // optional: (element: string) => void;
        isElementSelected: Function, // optional: (element: string) => boolean;
    },
})
export default class GenericListChart extends Vue {
    public label!: string;
    public elements!: string[];

    public onElementClicked?: (element: string) => void;
    public isElementSelected?: (element: string) => boolean;

    private isElementedSelectedFallback(element: string): boolean {
        if (!this.isElementSelected) {
            return false;
        }
        return this.isElementSelected(element);
    }
}

</script>

<style lang="scss" scoped>
li.selected::before {
    content: '>';
}
li::before {
    content: '-';
}

.clickableList li {
    @apply cursor-pointer text-blue-900 hover:text-blue-700;
}

li {
    @apply my-1
}
li:nth-child(odd) {
    @apply bg-gray-50;
}
</style>
