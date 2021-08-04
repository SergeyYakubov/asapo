import { createRouter, createWebHistory, RouteRecordRaw } from "vue-router";
import Dashboard from "../views/Dashboard.vue";

// Possible lazy load with: component: () => import("../views/SomePage.vue"),
const routes: Array<RouteRecordRaw> = [
    {
        path: "/",
        name: "Dashboard",
        component: Dashboard,
    }
];

const router = createRouter({
    history: createWebHistory(),
    routes,
});

export default router;
