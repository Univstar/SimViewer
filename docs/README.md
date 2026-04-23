# Docs

This folder contains public-facing documentation and media assets referenced by the repository homepage.

## Recommended Layout

- `media/`: screenshots, GIFs, short videos, and other assets embedded in Markdown
- `data-format.md`: input dataset contract for SimViewer

## Media Guidance

For GitHub repositories, it is usually better to keep screenshots and short demo assets under `docs/media/` rather than in the repository root.

Benefits:

- the root stays focused on source code and top-level metadata
- README image paths stay stable and easy to manage
- future docs pages can reuse the same assets

Current media assets:

- `media/simviewer-interface.png`
- `media/droplet-animation.gif`
- `media/falling-animation.gif`

GitHub renders images directly in Markdown. The repository README therefore uses GIF previews for inline layout.
