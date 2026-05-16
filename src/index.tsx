/* @refresh reload */
import 'solid-devtools';
import {render} from 'solid-js/web';
import App from './App';
import {Router} from '@solidjs/router';
import {routes} from './routes';

const root = document.getElementById('root');

if (import.meta.env.DEV && !(root instanceof HTMLElement)) {
    throw new Error(
        'Root element not found. Did you forget to add it to your index.html? Or maybe the id attribute got misspelled?',
    );
}

render(
    () => <Router root={(props) => <App>{props.children}</App>}>{routes}</Router>,
    root,
);
