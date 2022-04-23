export const ControllerRouteBases = {
    FRONT: "/",
    TRANSMIT: "/transmit",
    RECEIVE: "/receive",
};

export const FrontRoutes = {
    FRONT: "",
    ABOUT: "about",
    SETTINGS: "settings",
};

export const TransmitRoutes = {
    CONTROL: "control",
    CHANCHECK: "chan_check",
    FX: "fx",
};

export const ReceiveRoutes = {
    LEVELS: "levels",
};

export const Links = {
    FRONT_FRONT: `${ControllerRouteBases.FRONT}`,
    FRONT_ABOUT: `/${FrontRoutes.ABOUT}`,
    FRONT_SETTINGS: `/${FrontRoutes.SETTINGS}`,
    TRANSMIT_CONTROL: `${ControllerRouteBases.TRANSMIT}/${TransmitRoutes.CONTROL}`,
    TRANSMIT_CHANCHECK: `${ControllerRouteBases.TRANSMIT}/${TransmitRoutes.CHANCHECK}`,
    TRANSMIT_FX: `${ControllerRouteBases.TRANSMIT}/${TransmitRoutes.FX}`,
    RECEIVE_LEVELS: `${ControllerRouteBases.RECEIVE}/${ReceiveRoutes.LEVELS}`,
};
