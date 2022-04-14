import "./ThirdPartyPage.scss";

interface LicenseInfo {
    name: string;
    licenseText: string;
}

const licenses: LicenseInfo[] = [
    {
        name: "Boost",
        licenseText: require("./external_licenses/boost.txt"),
    },
    {
        name: "Bootstrap",
        licenseText: require("bootstrap/LICENSE"),
    },
    {
        name: "clsx",
        licenseText: require("clsx/license"),
    },
    {
        name: "Crow",
        licenseText: require("./external_licenses/crow.txt"),
    },
    {
        name: "EtcPal",
        licenseText: require("./external_licenses/etcpal.txt"),
    },
    {
        name: "{fmt}",
        licenseText: require("./external_licenses/fmtlib.txt"),
    },
    {
        name: "Protocol Buffers",
        licenseText: require("./external_licenses/protobuf.txt"),
    },
    {
        name: "protoc-gen-ts",
        licenseText: require('./external_licenses/protoc-gen-ts.txt'),
    },
    {
        name: "qr-code-generator",
        licenseText: require('./external_licenses/qr-code-generator.md'),
    },
    {
        name: "React",
        licenseText: require("react/LICENSE"),
    },
    {
        name: "React-Bootstrap",
        licenseText: require("react-bootstrap/LICENSE"),
    },
    {
        name: "react-dom",
        licenseText: require("react-dom/LICENSE"),
    },
    {
        name: "react-router-dom",
        licenseText: require("react-router-dom/LICENSE.md"),
    },
    {
        name: "react-scripts",
        licenseText: require("react-scripts/LICENSE"),
    },
    {
        name: "sACN",
        licenseText: require("./external_licenses/sacn.txt"),
    },
    {
        name: "Sass",
        licenseText: require("./../../node_modules/sass/LICENSE"),
    },
    {
        name: "spdlog",
        licenseText: require("./external_licenses/spdlog.txt"),
    },
    {
        name: "TypeScript",
        licenseText: require("./../../node_modules/typescript/LICENSE.txt"),
    },
];

export default function ThirdPartyPage() {
    return (
        <>
            <h1>3rd Party Software</h1>
            {licenses.map(license => (
                <License key={license.name} license={license}/>
            ))}
        </>
    );
}

function License(props: { license: LicenseInfo }) {
    const {license} = props;

    return (
        <div className="msacn-license">
            <h2>{license.name}</h2>
            <pre>{license.licenseText}</pre>
        </div>
    );
}
