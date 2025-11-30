import {defineConfig, loadEnv} from 'vite';
import react from '@vitejs/plugin-react';

// https://vitejs.dev/config/
export default defineConfig(({mode}) => {
    // See https://vite.dev/config/#using-environment-variables-in-config
    const env = loadEnv(mode, process.cwd(), '')
    return {
        plugins: [react()],
        build: {
            emptyOutDir: true,
            outDir: env.WEBUI_BUILD_DIR_REL,
        }
    };
});
