import { lazy } from 'solid-js';
import type { RouteDefinition } from '@solidjs/router';
import LINKS from "./links";

const ROUTES: RouteDefinition[] = [
  {
    path: LINKS.front_home,
    component: lazy(() => import("./pages/front/Home")),
  },
  {
    path: '**',
    component: lazy(() => import("./errors/404")),
  },
];

export default ROUTES;
