- Status: Proposed
- Date: 2023-05-29
- Author: [@lesleyjanenorton](https://github.com/lesleyjanenorton)
- RFC PR: [#6981](https://github.com/mozilla-mobile/mozilla-vpn-client/pull/6982)
- Implementation GitHub issue: Not applicable. Merging [#6981](https://github.com/mozilla-mobile/mozilla-vpn-client/pull/6982) implements the proposal.


## Summary
Introducing a process for proposing, reviewing, and documenting decisions with high-impact implications for our codebase.

## Motivation

RFCs authored by this team are currently submitted and reviewed in a number of places, namely Confluence and Google Docs.

Several challenges have been noted with this practice:
- Docs are accessible to Mozillians only
- Platform constraints make asynchronous discussions difficult and impossible to reference later (Confluence) 
- Uneven awareness and ability for interested parties to participate in decisions as they're being made (Google Docs) 

We need a more inclusive, more collaborative process for making decisions that impact our codebase.

## Proposed Solution

RFCs will be submitted and reviewed as pull requests in this repository. If approved, they'll be stored as documentation.

**Salient points in favor**
 - Viewable by all, not just Mozilla-internals
 - Closer to the code, easier to find
 - In keeping with how other teams do it (TODO link evidence)

## Guide-level explanation

The high-level process to create an RFC is as follows:

* Create an RFC document (like this one, in Markdown).
* Open a pull request for the RFC document and prefix the title of the PR with `[RFC]`.
* File an issue requesting feedback on the pull request.

During the lifetime of an RFC:

* Discussions happen asynchronously on the pull request; anyone may engage.
* Integrate feedback, build consensus, get stakeholder approval. All comments, not just those of the stakeholder, should be addressed and resolved.

After the discussion phase has concluded:

* If consensus is reached, the RFC is considered "accepted".
* If consensus is not reached, the RFC is considered "rejected".
* The status of the RFC is updated accordingly and is merged into the repository for documentation purposes.

Note:
* _Consensus_ in the bullets above should be understood as the general support of the team and stakeholder buy-in.
* Rejected RFC proposals may be revived should the requirements change in the future.

Authors may implement and submit the feature as a pull request once the RFC is accepted.

## Drawbacks
- I can't think of any

## Questions
- About nomenclature: Are we ok calling these 'RFCs'? Firefox Monitor and Firefox Relay refer to these as ADRs but 'RFC' strikes me as more to the point.
- About templates: I'm in favor of including an RFC template as an example but I'd prefer we not be overly prescriptive about it, at least early on.
- About documentation living elsewhere: What to do about design documents currently housed in Confluence, if anything? Are there documents we feel must be migrated over to GitHub and stored as RFCs? (Does not require a decision now) 

## Prior art
[Rust RFCs - Active RFC List](https://rust-lang.github.io/rfcs/)

[Firefox Android Components RFCs](https://github.com/pocmo/android-components/tree/master/docs/rfcs)

[Firefox Monitor ADRs](https://github.com/mozilla/blurts-server/tree/main/docs/adr)
