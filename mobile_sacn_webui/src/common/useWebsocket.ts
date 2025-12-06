import reactUseWebSocket, {Options as ReactUseWebSocketOptions, ReadyState} from "react-use-websocket";
import {useCallback, useEffect} from "react";
import {localStorageGet, LocalStorageItem} from "./localStorage.ts";

const serverOrigin = (() => {
    // Allow overriding the backend URL during development so the frontend can run from Vite's server.
    const serverOverride = localStorageGet(LocalStorageItem.SERVER, null);
    return serverOverride ?? document.location.origin;
})();

/**
 * Create a websocket connection to the host.
 *
 * @param protocol A string matching a path defined in HttpServer::HttpServer after `/ws` (e.g. where
 * HttpServer::HttpServer defines a websocket route for `/ws/ChanCheck`, pass `ChanCheck` here).
 * @param options Additional websocket options.
 */
export default function useWebsocket(protocol: string, options: Partial<ReactUseWebSocketOptions> = {}): ReturnType<typeof reactUseWebSocket> {
    const websocketOptions: ReactUseWebSocketOptions = {
        // Attempt to reconnect every second.
        reconnectInterval: 1000,
        // Reload the entire document if failed to reconnect after a minute.
        reconnectAttempts: 60,
        onReconnectStop: useCallback(() => window.location.reload(), []),
        shouldReconnect: useCallback<NonNullable<ReactUseWebSocketOptions["shouldReconnect"]>>((closeEvent) => {
            return !closeEvent.wasClean;
        }, []),
        retryOnError: true,
        // We're on a local network, so use aggressive heartbeats.
        heartbeat: localStorageGet(LocalStorageItem.ENABLE_HEARTBEAT, {
            timeout: 30000,
            interval: 5000,
        }),
        ...options,
    };
    return reactUseWebSocket(useWebsocketUrl(protocol), websocketOptions);
}

function useWebsocketUrl(protocol: string) {
    return useCallback(async () => {
        // Any errors thrown from this process are handled by useWebsocket.
        const response = await fetch(`${serverOrigin}/ws_url`, {headers: {"Content-Type": "text/plain"}});
        if (!response.ok) {
            throw new Error(`Code ${response.status} (${response.statusText}) while getting Websocket URL.`);
        }
        const wsUrl = await response.text();

        return `${wsUrl}/${protocol}`;
    }, [protocol]);
}
