import {Message} from "google-protobuf";
import {useCallback, useEffect, useState} from "react";

type SessionConnectHandler = (e: Event) => void;
type SessionMessageHandler<T extends Message> = (message: T, e: MessageEvent<Blob>) => void;
type SessionDisconnectHandler = (e: Event) => void;

const enum SessionCode {
    CLOSE_NORMAL = 1001,
}

const BASE_URL = `ws://${document.location.hostname}:5050/ws`;

type SessionHook = [
    (endpoint: string) => void,
    <T extends Message>(message: T) => void,
    () => void
]

export default function useSession<T extends Message>(messageType: typeof Message, onOpen: SessionConnectHandler, onMessage: SessionMessageHandler<T>, onClose: SessionDisconnectHandler): SessionHook {
    const [session, setSession] = useState<WebSocket | null>(null);

    // Set websocket event handlers.
    useEffect(() => {
        if (!session) {
            return;
        }
        session.onopen = onOpen;
        return () => {
            session.onopen = null;
        };
    }, [session, onOpen]);
    const messageDeserializer = useCallback((e: MessageEvent<Blob>) => {
        e.data.arrayBuffer().then((buf: ArrayBuffer) => {
            onMessage(messageType.deserializeBinary(new Uint8Array(buf)) as T, e);
        });
    }, [messageType, onMessage]);
    useEffect(() => {
        if (!session) {
            return;
        }
        session.onmessage = messageDeserializer;
        return () => {
            session.onmessage = null;
        };
    }, [session, messageDeserializer]);
    useEffect(() => {
        if (!session) {
            return;
        }
        session.onclose = onClose;
        return () => {
            session.onclose = null;
        };
    }, [session, onClose]);

    // Create websocket hooks.
    const connect = useCallback((endpoint: string) => {
        if (session === null) {
            const websocket = new WebSocket(`${BASE_URL}/${endpoint}`);
            setSession(websocket);
        }
    }, [session]);

    const sendMessage = <T extends Message>(message: T) => {
        if (!session) {
            console.warn("Can't send websocket message when websocket not connected.");
            return;
        }
        session.send(message.serializeBinary());
    };

    const close = useCallback(() => {
        if (!session) {
            console.warn("Can't close nonexistent websocket.");
            return;
        }
        if (session.readyState === session.OPEN) {
            session.close(SessionCode.CLOSE_NORMAL);
            setSession(null);
        }
    }, [session]);

    return [connect, sendMessage, close];
}
