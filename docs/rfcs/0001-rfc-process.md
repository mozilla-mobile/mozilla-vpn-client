- Status: Proposed
- Date: 2023-05-23
- RFC PR: TODO


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

* Create an RFC document (like this one). (TODO: templates, maybe)
* Open a pull request for the RFC document.
* Ask for feedback on the pull request.

During the lifetime of an RFC:

* Discussions happen asynchronously on the pull request; anyone may engage.
* Build consensus (TODO: agreement?) and integrate feedback.

After the discussion phase has concluded:

* If consensus(TODO: agreement?) is reached, the RFC is considered "accepted" and merged into the repository for documentation purposes.
* If consensus(TODO: agreement?) is not reached, the RFC is considered "rejected" and the pull request is closed. Rejected RFC proposals may be revived should the requirements change in the future.

Authors may implement and submit the feature as a pull request once the RFC is accepted.

## Drawbacks
- (?)

## Questions
- About nomenclature: Are we ok calling these 'RFCs'? Firefox Monitor and Firefox Relay refer to these as ADRs but 'RFC' strikes me as more to the point.
- About templates: I'm in favor of including an RFC template as an example but I'd prefer we not be overly prescriptive about it, at least early on.
- About documentation living elsewhere: What to do about design documents currently housed in Confluence, if anything? Are there documents we feel must be migrated over to GitHub and stored as RFCs? (Does not require a decision now) 

## Prior art
[Rust RFCs - Active RFC List](https://rust-lang.github.io/rfcs/)

[Firefox Android Components RFCs](https://github.com/pocmo/android-components/tree/master/docs/rfcs)

[Firefox Monitor ADRs](https://github.com/mozilla/blurts-server/tree/main/docs/adr)
