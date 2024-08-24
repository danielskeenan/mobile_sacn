import {StrictMode} from 'react';
import {createRoot} from 'react-dom/client';
import {createBrowserRouter, RouterProvider} from "react-router-dom";
import {Links} from "./routes.ts";
import App from "./App.tsx";

const router = createBrowserRouter([
    {
        path: Links.FRONT_FRONT,
        element: <App/>,
        children: [
            {
                index: true,
                lazy: () => import('./front/FrontPage.tsx'),
            },
        ],
    },
]);

createRoot(document.getElementById('root')!).render(
  <StrictMode>
      <RouterProvider router={router}/>
  </StrictMode>,
)
