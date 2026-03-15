const serverOrigin = (() => {
    // Allow overriding the backend URL during development so the frontend can run from Vite's server.
    const serverOverride = localStorage.getItem("server");
    return serverOverride ?? document.location.origin;
})();

export default function wsUrl(protocol: string) {
    return `${serverOrigin}/ws/${protocol}`
}
