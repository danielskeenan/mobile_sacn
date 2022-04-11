export const ControllerRouteBases = {
    FRONT: "/",
    TRANSMIT: "/transmit",
};

export const FrontRoutes = {
    FRONT: "",
    ABOUT: "about",
};

export const TransmitRoutes = {
    CONTROL: "control",
    CHANCHECK: "chan_check",
    FX: "fx",
};

export const Links = {
    FRONT_FRONT: `${ControllerRouteBases.FRONT}`,
    FRONT_ABOUT: `/${FrontRoutes.ABOUT}`,
    TRANSMIT_CONTROL: `${ControllerRouteBases.TRANSMIT}/${TransmitRoutes.CONTROL}`,
    TRANSMIT_CHANCHECK: `${ControllerRouteBases.TRANSMIT}/${TransmitRoutes.CHANCHECK}`,
    TRANSMIT_FX: `${ControllerRouteBases.TRANSMIT}/${TransmitRoutes.FX}`,
};
