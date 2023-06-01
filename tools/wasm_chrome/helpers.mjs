/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

export async function getGithubPaginatedData(octokit, request, options) {
    let result = [];
    let page = 1;
    while (true) {
        const response = await octokit.request(
            request,
            {
                ...options,
                per_page: 100, // This is the maximum.
                page
            },
        );
        console.log(`RateLimit -> ${response.headers["x-ratelimit-remaining"]}`);

        if (response.status != 200) {
            console.error(response);
            Deno.exit(-1);
        }

        if (response.data.length === 0) {
            break;
        }

        result = [...result, ...response.data];
        page++;
    }

    return result;
}
