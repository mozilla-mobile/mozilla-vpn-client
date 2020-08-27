build_wireguard_go.target = $$OUT_PWD/.buildfile
build_wireguard_go.commands = cd $$PWD/wireguard-go && make
QMAKE_EXTRA_TARGETS += build_wireguard_go
PRE_TARGETDEPS += $$OUT_PWD/.buildfile

TEMPLATE = aux
