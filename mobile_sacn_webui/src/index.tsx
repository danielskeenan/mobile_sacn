/* @refresh reload */
import {AppContextProvider} from "@/common/AppContext";
import {Router} from "@solidjs/router";
import {render} from "solid-js/web";
import App from "./App";
import ROUTES from "./routes";

render(() => (
    <AppContextProvider>
        <Router root={App}>{ROUTES}</Router>
    </AppContextProvider>
), document.getElementById("root") as HTMLElement);
