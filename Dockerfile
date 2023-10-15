ARG VERSION
FROM postgres:${VERSION}-bullseye

ARG VERSION

RUN : \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        build-essential \
        gcc \
        postgresql-server-dev-$VERSION \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/local/lib/funcs

COPY ./pg_solpos.c .
COPY ./solpos* .

RUN : \
    && cc -fPIC -Werror -Wall -c solpos.c \
    && cc -fPIC -Werror -Wall -c pg_solpos.c -lm -I /usr/include/postgresql/${VERSION}/server \
    && cc -shared -o pg_solpos.so pg_solpos.o solpos.o \
    && find . -name "*solpos*" | grep -v pg_solpos.so | xargs rm
