FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y --no-install-recommends build-essential libncurses-dev libcrypt-dev && rm -rf /var/lib/apt/lists/*
WORKDIR /bbs
COPY src/ src/
RUN make -C src all check BBSACCOUNT=root
COPY root/ root/
COPY docs/ docs/
RUN mkdir -p root/bin root/boards root/mail root/tmp root/msgtmp root/chat root/vote root/signatures root/plans \
    && cp src/bbs src/bbs.chatd src/bbsrf src/SaveToCabinet src/cntuser root/bin/ \
    && useradd --uid 10001 --no-create-home bbs && chown -R bbs:bbs root
ENV BBSHOME=/bbs/root TERM=vt100 LANG=C.UTF-8
WORKDIR /bbs/root
USER bbs
CMD ["/bbs/src/bbs"]
