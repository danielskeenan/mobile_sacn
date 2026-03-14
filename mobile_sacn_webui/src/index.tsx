/* @refresh reload */
import 'solid-devtools';

import {render} from 'solid-js/web';
import App from './App';
import {Router} from '@solidjs/router';
import ROUTES from './routes';
import setPageTitle from "./common/setPageTitle";

setPageTitle();

render(()=>(
    <Router root={App}>{ROUTES}</Router>
), document.getElementById("root") as HTMLElement)
