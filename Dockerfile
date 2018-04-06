FROM ubuntu:16.04

RUN apt-get update && apt-get install -y g++ gcc cmake pkg-config git curl libcurl3-dev sqlite3 libglib2.0-dev libsqlite3-dev

WORKDIR /opt

RUN git clone https://github.com/sahib/glyr.git

WORKDIR /opt/glyr

RUN mkdir build

WORKDIR /opt/glyr/build

RUN cmake .. && make

RUN apt-get -y purge g++ gcc cmake pkg-config git && apt-get clean && rm -rf /var/lib/apt

RUN ln -s /opt/glyr/build/bin/glyrc /usr/bin/

CMD /opt/glyr/build/bin/glyrc