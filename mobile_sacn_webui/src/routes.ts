import { lazy } from 'solid-js';
import type { RouteDefinition } from '@solidjs/router';
import LINKS from "./links";

const ROUTES: RouteDefinition[] = [
  {
    path: LINKS.front_home,
    component: lazy(() => import("./pages/front/HomePage")),
  },

  {
    path: LINKS.transmit_chancheck,
    component: lazy(() => import("./pages/transmit/ChannelCheckPage")),
  },

  {
    path: LINKS.transmit_levels,
    component: lazy(() => import("./pages/transmit/TransmitLevelsPage")),
  },

  {
    path: LINKS.receive_levels,
    component: lazy(() => import("./pages/receive/levels/ReceiveLevelsPage")),
  },

  {
    path: '**',
    component: lazy(() => import("./errors/404")),
  },
];

export default ROUTES;
