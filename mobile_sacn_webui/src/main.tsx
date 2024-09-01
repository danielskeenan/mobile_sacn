import {StrictMode} from 'react';
import {createRoot} from 'react-dom/client';
import {createBrowserRouter, RouterProvider} from "react-router-dom";
import {Links} from "./routes.ts";
import App from "./App.tsx";
import {Loading} from "./common/components/Loading.tsx";

const router = createBrowserRouter([
    {
        path: Links.FRONT_FRONT,
        element: <App/>,
        children: [
            {
                index: true,
                lazy: () => import('./front/FrontPage.tsx'),
            },
            {
                path: Links.FRONT_SETTINGS,
                lazy: () => import('./front/SettingsPage.tsx'),
            },
            {
                path: Links.TRANSMIT_CHANCHECK,
                lazy: () => import('./transmit/chancheck/ChanCheck.tsx'),
            },
            {
                path: Links.TRANSMIT_LEVELS,
                lazy: () => import('./transmit/levels/Levels.tsx'),
            },
            {
                path: Links.RECEIVE_LEVELS,
                lazy: () => import('./receive/levels/Levels.tsx'),
            },
        ],
    },
]);

createRoot(document.getElementById('root')!).render(
    <StrictMode>
        <RouterProvider router={router} fallbackElement={<Loading/>}/>
    </StrictMode>,
);
