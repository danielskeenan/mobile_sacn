import "./ThirdPartyPage.scss";

interface LicenseInfo {
    name: string;
    licenseText: string;
}

const licenses: LicenseInfo[] = [
    {
        name: "Boostrap",
        licenseText: require("bootstrap/LICENSE"),
    },
    {
        name: "clsx",
        licenseText: require("clsx/license"),
    },
    {
        name: "protoc-gen-ts",
        licenseText: `Copyright 2020 Sahin Yort (thesayyn)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.`,
    },
    {
        name: "react",
        licenseText: require("react/LICENSE"),
    },
    {
        name: "react-bootstrap",
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
        name: "sass",
        licenseText: require("./../../node_modules/sass/LICENSE"),
    },
    {
        name: "typescript",
        licenseText: require("./../../node_modules/typescript/LICENSE.txt"),
    }
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
