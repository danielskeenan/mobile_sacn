export const ControllerRouteBases = {
    FRONT: "/",
    TRANSMIT: "/transmit",
    RECEIVE: "/receive",
};

export const FrontRoutes = {
    FRONT: "",
    SETTINGS: "settings",
};

export const TransmitRoutes = {
    LEVELS: "levels",
    CHANCHECK: "chan_check",
    FX: "fx",
};

export const ReceiveRoutes = {
    LEVELS: "levels",
};

export const Links = {
    FRONT_FRONT: `${ControllerRouteBases.FRONT}`,
    FRONT_SETTINGS: `/${FrontRoutes.SETTINGS}`,
    TRANSMIT_LEVELS: `${ControllerRouteBases.TRANSMIT}/${TransmitRoutes.LEVELS}`,
    TRANSMIT_CHANCHECK: `${ControllerRouteBases.TRANSMIT}/${TransmitRoutes.CHANCHECK}`,
    TRANSMIT_FX: `${ControllerRouteBases.TRANSMIT}/${TransmitRoutes.FX}`,
    RECEIVE_LEVELS: `${ControllerRouteBases.RECEIVE}/${ReceiveRoutes.LEVELS}`,
};
