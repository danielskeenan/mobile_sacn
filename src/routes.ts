import {lazy} from 'solid-js';
import type {RouteDefinition} from '@solidjs/router';
import Home from './pages/Home';
import LINKS from "@/links";

export const routes: RouteDefinition[] = [
  {
    path: LINKS.home,
    component: Home,
  },
  {
    path: '**',
    component: lazy(() => import('./errors/404')),
  },
];
