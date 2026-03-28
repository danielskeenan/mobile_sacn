/* @refresh reload */
import 'solid-devtools';
import {AppContextProvider} from "@/common/AppContext";

import {render} from 'solid-js/web';
import App from './App';
import {Router} from '@solidjs/router';
import ROUTES from './routes';
import setPageTitle from "./common/setPageTitle";

setPageTitle();

render(()=>(
    <AppContextProvider>
        <Router root={App}>{ROUTES}</Router>
    </AppContextProvider>
), document.getElementById("root") as HTMLElement)
