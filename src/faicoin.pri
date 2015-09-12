INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD/leveldb/include
INCLUDEPATH += $$PWD/leveldb/helpers/memenv
INCLUDEPATH += $$PWD/secp256k1/include
INCLUDEPATH += /usr/include/boost
DEPENDPATH += /usr/include/boost
INCLUDEPATH += /usr/include/openssl
INCLUDEPATH += /usr/include
DEPENDPATH += /usr/include
win32 {
INCLUDEPATH += C:/deps/boost_1_57_0
INCLUDEPATH += C:/deps/openssl-1.0.1l/include
INCLUDEPATH += C:/deps/sqlite-autoconf-3081002
INCLUDEPATH += C:/deps/gmp-6.0.0
DEPENDPATH += C:/deps/openssl-1.0.1l/include/openssl
DEPENDPATH += C:/deps/miniupnpc
}
LIBS += $$PWD/libbitcoin_server.a
LIBS += $$PWD/libbitcoin_wallet.a
LIBS += $$PWD/libbitcoin_common.a
LIBS += $$PWD/libbitcoin_cli.a
LIBS += $$PWD/libbitcoin_util.a
LIBS += $$PWD/crypto/libbitcoin_crypto.a
LIBS += $$PWD/univalue/libbitcoin_univalue.a
LIBS += $$PWD/leveldb/libleveldb.a
LIBS += $$PWD/leveldb/libmemenv.a
LIBS += $$PWD/secp256k1/.libs/libsecp256k1.a

unix {
LIBS += \
  -lboost_date_time \
  -lboost_filesystem \
  -lboost_program_options \
  -lboost_regex \
  -lboost_signals \
  -lboost_thread \
  -lboost_system \
  -lboost_chrono\
  -lboost_program_options \
  -lboost_unit_test_framework \
  -lboost_regex \
  -lssl \
  -lcrypto \
  -lminiupnpc \
  -lprotobuf \
  -lpthread \
  -lsecp256k1 \
  -lsqlite3 \
  -lanl
}
win32 {
LIBS += C:/deps/openssl-1.0.1l/libssl.a
LIBS += C:/deps/openssl-1.0.1l/libcrypto.a
LIBS += C:/deps/boost_1_57_0/stage/lib/libboost_system-mgw49-mt-s-1_57.a
LIBS += C:/deps/boost_1_57_0/stage/lib/libboost_thread-mgw49-mt-s-1_57.a
LIBS += C:/deps/boost_1_57_0/stage/lib/libboost_filesystem-mgw49-mt-s-1_57.a
LIBS += C:/deps/boost_1_57_0/stage/lib/libboost_chrono-mgw49-mt-s-1_57.a
LIBS += C:/deps/boost_1_57_0/stage/lib/libboost_program_options-mgw49-mt-s-1_57.a

LIBS += -lole32
LIBS += -loleaut32
LIBS += -luuid
LIBS += -lshlwapi
LIBS += -lssp

LIBS += C:/deps/miniupnpc/libminiupnpc.a
LIBS += C:/deps/gmp-6.0.0/.libs/libgmp.a
LIBS += C:/deps/sqlite-autoconf-3081002/.libs/libsqlite3.a
LIBS += C:/deps/libpng-1.6.16/.libs/libpng.a
LIBS += -lgdi32
LIBS += -liphlpapi
LIBS += -lws2_32
LIBS += -lwsock32
}




