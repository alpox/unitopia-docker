FROM debian:bookworm@sha256:49ba348354a28e39c70beffd6cf43bdb8d55d81ce4b746b0428717d054b8bbc4

RUN apt-get update \
 && apt-get install -y --no-install-recommends build-essential ca-certificates git bison autoconf autogen automake wget pkg-config libgcrypt20-dev libgnutls28-dev libsqlite3-dev python3-dev libxml2-dev zlib1g-dev libpcre3-dev libc-ares-dev python3-hunspell hunspell-de-de locales \
 && echo "de_DE.UTF-8 UTF-8" > /etc/locale.gen \
 && dpkg-reconfigure --frontend=noninteractive locales \
 && update-locale LANG=de_DE.UTF-8 \
 && git clone -b unitopia https://github.com/amotzkau/ldmud.git \
 && cd ldmud && git checkout 591454a81df608bf24185c6389affb1862a28c39 \
 && cd src && ./autogen.sh && settings/unitopia --prefix=/usr/local --disable-use-ipv6 && make install-driver install-utils && cd ../.. \
 && rm -rf ldmud \
 && mkdir /mud && cd /mud \
 && wget -O - https://www.unitopia.de/pub/UNItopia/mudlib.tar.gz | tar -zxf - \
 && wget -O - https://www.unitopia.de/pub/UNItopia/muddocs.tar.gz | tar -zxf - \
 && wget -O - https://www.unitopia.de/pub/UNItopia/python-support.tar.gz | tar -zxf - \
 && apt-get clean \
 && apt-mark manual libgnutls30 libsqlite3-0 libpython3.11 libxml2 libpcre3 \
 && apt-get remove --purge -y build-essential ca-certificates git bison autoconf autogen automake wget pkg-config libgcrypt20-dev libgnutls28-dev libsqlite3-dev python3-dev libxml2-dev zlib1g-dev libpcre3-dev \
 && apt-get autoremove -y \
 && lib/doc/driver/setup_mudlib

ENV LANG de_DE.UTF-8
ENV LC_ALL de_DE.UTF-8

# Save the plain mudlib for copying into the volume.
RUN tar -zcf /mud/lib.tar.gz -C /mud/lib .

ADD driver.sh /usr/local/bin/

EXPOSE 3333 3335/udp
VOLUME /mud/lib

CMD [ "/usr/local/bin/driver.sh" ]
