#!/usr/bin/env node

import {Command} from 'commander';
import {Octokit} from "octokit";
import tmp from "tmp";
import path from "node:path";
import * as fs from "node:fs";
import {Readable} from "node:stream";
import {finished} from "node:stream/promises";
import AdmZip from "adm-zip";

const GH_REPO = {
    owner: 'danielskeenan',
    repo: 'mobile_sacn',
}

tmp.setGracefulCleanup();

const octokit = new Octokit({});

async function getRelease(tagName?: string) {
    let release;
    if (!tagName) {
        release = await octokit.rest.repos.getLatestRelease(GH_REPO);
    } else {
        release = await octokit.rest.repos.getReleaseByTag({...GH_REPO, tag: tagName});
    }
    if (release.status !== 200) {
        throw new Error(`Error fetching release: ${release.status}`);
    }
    return release.data;
}

function getDocPackageUrl(releaseData: Awaited<ReturnType<typeof getRelease>>) {
    for (const assetData of releaseData.assets) {
        if (!assetData.name.startsWith('mobile_sacn_doc')) {
            continue;
        }
        return assetData.browser_download_url;
    }

    throw new Error(`Could not load doc package for release: ${releaseData.name}`);
}

async function downloadDocs(docPackageUrl: string, outPath: string) {
    // Download ZIP file.
    const tmpDir = tmp.dirSync({unsafeCleanup: true});
    const resp = await fetch(docPackageUrl);
    if (!resp.ok || !resp.body) {
        throw new Error(`Could not download doc: ${resp.statusText}`);
    }
    const zipPath = path.resolve(tmpDir.name, "mobile_sacn_doc.zip");
    const out = fs.createWriteStream(zipPath, {flags: 'w'});
    await finished(Readable.fromWeb(resp.body).pipe(out));
    out.close();

    // Extract to out path.
    const zip = new AdmZip(zipPath);
    zip.extractEntryTo("html/", outPath, false, true);
}

const program = new Command();
program.name('fetch-docs')
    .description('Fetch a doc package from GitHub release')
    .option('-r, --release <tag>', 'Release tag name')
    .argument('<out-path>', 'Path to place extracted files')
program.parse();

const release = await getRelease(program.opts().release);
console.log(`Downloading docs for release ${release.name}`);
const docPackageUrl = getDocPackageUrl(release);
await downloadDocs(docPackageUrl, program.args[0]);
console.log('Complete!');