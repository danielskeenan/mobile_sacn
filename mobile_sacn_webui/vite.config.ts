import {defineConfig, loadEnv} from 'vite';
import solidPlugin from 'vite-plugin-solid';
import devtools from 'solid-devtools/vite';
import path from "node:path";

export default defineConfig(({mode}) => {
    // See https://vite.dev/config/#using-environment-variables-in-config
    const env = loadEnv(mode, process.cwd(), '')
    return {
        plugins: [devtools(), solidPlugin()],
        server: {
            port: 3000,
        },
        build: {
            target: 'esnext',
            emptyOutDir: true,
            outDir: env.WEBUI_BUILD_DIR_REL,
        },
        resolve: {
            alias: {
                "@": path.resolve(__dirname, "./src/"),
            },
        }
    };
});
