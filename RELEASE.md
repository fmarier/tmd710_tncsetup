# Making a new release

0. Bump the copyright year in `README.md` and `tmd710_tncsetup.c` and commit if needed.
1. Bump the version number in `tmd710_tncsetup.c` **without committing**.
2. Run `make dist` to commit and sign.
3. Run `git push --follow-tags` to push code and tags to GitHub.
4. Go to <https://github.com/fmarier/tmd710_tncsetup/tags> to download the latest tarball.
5. Sign it using `gpg --armor --sign --detach-sig ~/download/tmd710_tncsetup-VERSION.tar.gz`.
6. Click on *...* an then *Create release* from <https://github.com/fmarier/tmd710_tncsetup/tags>.
7. Use `VERSION release` as the title and summarize the changes.
8. Attach `tmd710_tncsetup-VERSION.tar.gz.asc`.
