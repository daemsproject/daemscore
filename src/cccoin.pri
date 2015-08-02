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




