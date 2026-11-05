# lxdyn deploy image. Build it with `mise run deploy:image`, which stages the Nix-built static
# binaries (`nix build .#xdyn-deploy`) into build/scratch/deploy/ first — this file deliberately does
# no compiling, so the image ships the binaries Nix built and checked, not a second build of them.

FROM scratch

COPY build/scratch/deploy/ /usr/bin/

LABEL org.opencontainers.image.source=https://github.com/naval-group/lxdyn
LABEL org.opencontainers.image.title=lxdyn
LABEL org.opencontainers.image.description="lxdyn, the LOTUSim fork of xdyn: a time-domain ship simulator"
LABEL org.opencontainers.image.licenses=EPL-2.0

# These two labels are what makes a pulled image traceable with `inspect` alone. 
# deploy:image fills them from the flake; the defaults are what a plain `podman build .` gets.
ARG VERSION=dev
ARG REVISION=unknown
LABEL org.opencontainers.image.version=$VERSION
LABEL org.opencontainers.image.revision=$REVISION

# /data, because xdyn writes its .h5/.csv/.json next to the cwd
WORKDIR /data

ENTRYPOINT ["/usr/bin/xdyn"]
