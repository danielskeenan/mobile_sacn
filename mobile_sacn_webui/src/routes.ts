import { lazy } from 'solid-js';
import type { RouteDefinition } from '@solidjs/router';
import LINKS from "./links";

const ROUTES: RouteDefinition[] = [
  {
    path: LINKS.front_home,
    component: lazy(() => import("./pages/front/Home")),
  },

  {
    path: LINKS.transmit_chancheck,
    component: lazy(() => import("./pages/transmit/ChannelCheck")),
  },

  {
    path: LINKS.receive_levels,
    component: lazy(() => import("./pages/receive/levels/ReceiveLevels")),
  },

  {
    path: '**',
    component: lazy(() => import("./errors/404")),
  },
];

export default ROUTES;
