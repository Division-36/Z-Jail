FROM gcc:13-bookworm AS build
WORKDIR /src
COPY . .
RUN make && make -C tests setup
FROM debian:bookworm-slim
COPY --from=build /src/z_jail /usr/local/bin/
ENTRYPOINT ["z_jail"]
