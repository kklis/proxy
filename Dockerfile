FROM gcc

WORKDIR /usr/src/app/
COPY . /usr/src/app/

RUN make

ENTRYPOINT [ "./proxy", "-f" ]
