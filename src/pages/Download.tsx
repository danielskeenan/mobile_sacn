import {type Component, createResource, Index, type JSX, Match, Switch} from "solid-js";
import {Title} from "@solidjs/meta";
import {Alert, Container, Row, Spinner, Table} from "solid-bootstrap";
import {Octokit} from "octokit";
import {FaBrandsLinux, FaBrandsWindows} from "solid-icons/fa";
import {filesize} from "filesize";

const GH_REPO = {
    owner: 'danielskeenan',
    repo: 'mobile_sacn',
}

interface ReleaseAssetLabel {
    icon: JSX.Element;
    platform: string;
    format: string;
}

interface ReleaseAsset {
    label: ReleaseAssetLabel;
    url: string;
    size: number;
    checksum: string;
}

const ASSET_LABELS = new Map<RegExp, ReleaseAssetLabel>([
    [/-Windows\.msi$/, {icon: <FaBrandsWindows/>, platform: "Windows", format: "Installer"}],
    [/-Windows\.zip$/, {icon: <FaBrandsWindows/>, platform: "Windows", format: "Portable"}],
    [/_amd64\.deb$/, {icon: <FaBrandsLinux/>, platform: "Linux", format: "DEB Package"}],
    [/-Linux\.rpm$/, {icon: <FaBrandsLinux/>, platform: "Linux", format: "RPM Package"}],
    [/-Linux\.tar\.gz$/, {icon: <FaBrandsLinux/>, platform: "Linux", format: "Tarball"}],
]);

const fetchRelease = async () => {
    const octokit = new Octokit({});
    const resp = await octokit.rest.repos.getLatestRelease(GH_REPO);
    if (resp.status !== 200) {
        throw new Error(`Error fetching release: ${resp.status}`);
    }
    return resp.data;
}

const fetchDownloads = async (release: Awaited<ReturnType<typeof fetchRelease>>) => {
    const downloads: ReleaseAsset[] = [];
    for (const assetData of release.assets) {
        for (const [re, label] of ASSET_LABELS) {
            if (re.test(assetData.name)) {
                downloads.push({
                    label: label,
                    url: assetData.browser_download_url,
                    size: assetData.size,
                    checksum: assetData.digest,
                });
                break;
            }
        }
    }

    downloads.sort((lhs, rhs) => {
        if (lhs.label.platform != rhs.label.platform) {
            return lhs.label.platform.localeCompare(rhs.label.platform);
        }
        if (lhs.label.format != rhs.label.format) {
            return lhs.label.format.localeCompare(rhs.label.format);
        }
        return 0;
    })

    return downloads;
}

const fetchReleaseNotes = async (release: Awaited<ReturnType<typeof fetchRelease>>) => {
    const octokit = new Octokit({});
    const resp = await octokit.rest.markdown.render({
        text: release.body,
        mode: 'gfm',
        context: `${GH_REPO.owner}/${GH_REPO.repo}`
    });
    if (resp.status !== 200) {
        throw new Error(`Error rendering release notes: ${resp.status}`);
    }
    return resp.data;
}

const Download: Component = () => {
    const [release] = createResource(fetchRelease);
    const [downloads] = createResource(release, fetchDownloads);
    const [releaseNotes] = createResource(release, fetchReleaseNotes);

    return (
        <main>
            <Title>Mobile sACN | Download</Title>
            <Container>
                <Row>
                    <h1>Download</h1>
                    <Switch>
                        <Match when={downloads.loading}>
                            <Alert variant="info"><Spinner animation="border"/>&nbsp;Getting downloads...</Alert>
                        </Match>
                        <Match when={downloads.error}>
                            <Alert variant="danger">
                                Error loading downloads. Visit
                                <a href={`https://github.com/${GH_REPO.owner}/${GH_REPO.repo}/releases/latest`}>
                                    {`https://github.com/${GH_REPO.owner}/${GH_REPO.repo}/releases/latest`}
                                </a>
                                for a list of releases.
                            </Alert>
                        </Match>
                        <Match when={downloads()}>
                            <Table responsive="xl">
                                <thead>
                                <tr>
                                    <th>Platform</th>
                                    <th>Format</th>
                                    <th>Size</th>
                                    <th>Checksum</th>
                                </tr>
                                </thead>
                                <tbody>
                                <Index each={downloads()}>
                                    {(download) => (
                                        <tr>
                                            <td>{download().label.icon}&nbsp;{download().label.platform}</td>
                                            <td><a href={download().url}>{download().label.format}</a></td>
                                            <td>{filesize(download().size)}</td>
                                            <td>{download().checksum}</td>
                                        </tr>
                                    )}
                                </Index>
                                </tbody>
                            </Table>
                        </Match>
                    </Switch>
                </Row>
                <Row>
                    <h2>Release Notes</h2>
                    <Switch>
                        <Match when={releaseNotes.loading}>
                            <Alert variant="info"><Spinner animation="border"/>&nbsp;Getting release notes...</Alert>
                        </Match>
                        <Match when={releaseNotes.error}>
                            <Alert variant="danger">
                                Error loading release notes. Visit
                                <a href={`https://github.com/${GH_REPO.owner}/${GH_REPO.repo}/releases/latest`}>
                                    {`https://github.com/${GH_REPO.owner}/${GH_REPO.repo}/releases/latest`}
                                </a>
                                for a list of releases.
                            </Alert>
                        </Match>
                        <Match when={releaseNotes()}>
                            <div innerHTML={releaseNotes()}/>
                        </Match>
                    </Switch>
                </Row>
            </Container>
        </main>
    );
}

export default Download;