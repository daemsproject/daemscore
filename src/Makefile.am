DIST_SUBDIRS = secp256k1
AM_LDFLAGS = $(PTHREAD_CFLAGS) $(LIBTOOL_LDFLAGS)
AUTOMAKE_OPTIONS = subdir-objects


if EMBEDDED_LEVELDB
LEVELDB_CPPFLAGS += -I$(srcdir)/leveldb/include
LEVELDB_CPPFLAGS += -I$(srcdir)/leveldb/helpers/memenv
LIBLEVELDB += $(builddir)/leveldb/libleveldb.a
LIBMEMENV  += $(builddir)/leveldb/libmemenv.a

# NOTE: This dependency is not strictly necessary, but without it make may try to build both in parallel, which breaks the LevelDB build system in a race
$(LIBLEVELDB): $(LIBMEMENV)

$(LIBLEVELDB) $(LIBMEMENV):
	@echo "Building LevelDB ..." && $(MAKE) -C $(@D) $(@F) CXX="$(CXX)" \
	  CC="$(CC)" PLATFORM=$(TARGET_OS) AR="$(AR)" $(LEVELDB_TARGET_FLAGS) \
          OPT="$(CXXFLAGS) $(CPPFLAGS) -D__STDC_LIMIT_MACROS"
endif

BITCOIN_CONFIG_INCLUDES=-I$(builddir)/config
BITCOIN_INCLUDES=-I$(builddir) -I$(builddir)/obj $(BOOST_CPPFLAGS) $(LEVELDB_CPPFLAGS) $(CRYPTO_CFLAGS) $(SSL_CFLAGS)

BITCOIN_INCLUDES += -I$(srcdir)/secp256k1/include

LIBBITCOIN_SERVER=libbitcoin_server.a
LIBBITCOIN_WALLET=libbitcoin_wallet.a
LIBBITCOIN_COMMON=libbitcoin_common.a
LIBBITCOIN_CLI=libbitcoin_cli.a
LIBBITCOIN_UTIL=libbitcoin_util.a
LIBBITCOIN_CRYPTO=crypto/libbitcoin_crypto.a
LIBBITCOIN_UNIVALUE=univalue/libbitcoin_univalue.a
LIBBITCOINQT=qt/libbitcoinqt.a
LIBSECP256K1=secp256k1/libsecp256k1.la
LIBZCASH=libzcash.a

$(LIBSECP256K1): $(wildcard secp256k1/src/*) $(wildcard secp256k1/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

# Make is not made aware of per-object dependencies to avoid limiting building parallelization
# But to build the less dependent modules first, we manually select their order here:
EXTRA_LIBRARIES = \
  crypto/libbitcoin_crypto.a \
  libbitcoin_util.a \
  libbitcoin_common.a \
  univalue/libbitcoin_univalue.a \
  libbitcoin_server.a \
  libbitcoin_cli.a \
  libzcash.a
if ENABLE_WALLET
BITCOIN_INCLUDES += $(BDB_CPPFLAGS)
EXTRA_LIBRARIES += libbitcoin_wallet.a
endif

if BUILD_BITCOIN_LIBS
lib_LTLIBRARIES = libdaemsconsensus.la
LIBDAEMS_CONSENSUS=libdaemsconsensus.la
else
LIBDAEMS_CONSENSUS=
endif

bin_PROGRAMS =
TESTS =

if BUILD_BITCOIND
  bin_PROGRAMS += daemsd
endif

if BUILD_BITCOIN_UTILS
  bin_PROGRAMS += daems-cli daems-tx
endif

LIBZCASH_H = \
  zcash/IncrementalMerkleTree.hpp \
  zcash/NoteEncryption.hpp \
  zcash/Address.hpp \
  zcash/JoinSplit.hpp \
  zcash/Note.hpp \
  zcash/prf.h \
  zcash/Proof.hpp \
  zcash/util.h \
  zcash/Zcash.h

.PHONY: FORCE check-symbols check-security
# bitcoin core #
BITCOIN_CORE_H = \
  addrman.h \
  alert.h \
  amount.h \
  arith_uint256.h \
  asyncrpcoperation.h \
  asyncrpcqueue.h \
  base58.h \
  bloom.h \
  cc/cc.h \
  cc/content.h \  
  cc/link.h \
  cc/jsondb.h \
  chain.h \
  chainparams.h \
  chainparamsbase.h \
  chainparamsseeds.h \
  checkpoints.h \
  checkqueue.h \
  clientversion.h \
  coincontrol.h \
  coins.h \
  cc/domain.h \
 cc/contentutil.h\
  cc/settings.h\
 cc/filepackage.h\
  compat.h \
  compat/byteswap.h \
  compat/endian.h \
  compat/sanity.h \
  compressor.h \
  primitives/block.h \
  primitives/transaction.h \
  consensus/consensus.h \
  consensus/params.h \
  consensus/validation.h \
  core_io.h \ 
  crypto/scrypt.h \
  eccryptoverify.h \
  cc/ecminer.h \
  ecwrapper.h \
  hash.h \
  init.h \
  key.h \
  keystore.h \
  leveldbwrapper.h \
  cc/sqlitewrapper.h \
    cc/p2pservice.h \
  limitedmap.h \
  main.h \
  memusage.h \
  merkleblock.h \
  metrics.h \
  miner.h \
  mruset.h \
  netbase.h \
  net.h \
  noui.h \
  policy/fees.h \
  pow.h \
  protocol.h \
  pubkey.h \
  random.h \
  reverselock.h \
  rpcclient.h \
  rpcprotocol.h \
  rpcserver.h \
  scheduler.h \
  script/interpreter.h \
  script/script.h \
  script/sigcache.h \
  script/sign.h \
  script/standard.h \
  script/script_error.h \
  serialize.h \
  streams.h \
  support/allocators/secure.h \
  support/allocators/zeroafterfree.h \
  support/cleanse.h \
  support/pagelocker.h \
  sync.h \
  threadsafety.h \
  timedata.h \
  tinyformat.h \
  txdb.h \
  txmempool.h \
  ui_interface.h \
  uint256.h \
  uint252.h \
  undo.h \
  util.h \
  utilstrencodings.h \
  utilmoneystr.h \
  utiltime.h \
  validationinterface.h \
  version.h \
  wallet/asyncrpcoperation_sendmany.h \
  wallet/crypter.h \
  wallet/db.h \
  wallet/wallet.h \
  wallet/wallet_ismine.h \
  wallet/walletdb.h

JSON_H = \
  json/json_spirit.h \
  json/json_spirit_error_position.h \
  json/json_spirit_reader.h \
  json/json_spirit_reader_template.h \
  json/json_spirit_stream_reader.h \
  json/json_spirit_utils.h \
  json/json_spirit_value.h \
  json/json_spirit_writer.h \
  json/json_spirit_writer_template.h

obj/build.h: FORCE
	@$(MKDIR_P) $(builddir)/obj
	@$(top_srcdir)/share/genbuild.sh $(abs_top_builddir)/src/obj/build.h \
	  $(abs_top_srcdir)
libbitcoin_util_a-clientversion.$(OBJEXT): obj/build.h

# server: shared between bitcoind and bitcoin-qt
libbitcoin_server_a_CPPFLAGS = $(BITCOIN_INCLUDES) $(MINIUPNPC_CPPFLAGS)
libbitcoin_server_a_SOURCES = \
  sendalert.cpp \
  addrman.cpp \
  alert.cpp \
  alertkeys.h \
  asyncrpcoperation.cpp \
  asyncrpcqueue.cpp \
  bloom.cpp \
  chain.cpp \
  checkpoints.cpp \
  cc/ecminer.cpp \
  init.cpp \
  leveldbwrapper.cpp \
 cc/sqlitewrapper.cpp \
  cc/settings.cpp\
  cc/contentutil.cpp\
  cc/filepackage.cpp\
  main.cpp \
  cc/p2pservice.cpp \
  merkleblock.cpp \
  metrics.cpp \
  miner.cpp \
  net.cpp \
  noui.cpp \
  policy/fees.cpp \
  pow.cpp \
  rest.cpp \
  rpcblockchain.cpp \
  rpccontent.cpp \
  rpcjsondb.cpp \
  rpcmining.cpp \
  rpcmisc.cpp \
  rpcnet.cpp \
  rpcrawtransaction.cpp \
  rpcserver.cpp \
  rpcutil.cpp \
  script/sigcache.cpp \
  timedata.cpp \
  txdb.cpp \
  txmempool.cpp \
  validationinterface.cpp \
  $(JSON_H) \
  $(BITCOIN_CORE_H) \
  $(LIBZCASH_H)

# wallet: shared between bitcoind and bitcoin-qt, but only linked
# when wallet enabled
libbitcoin_wallet_a_CPPFLAGS = $(BITCOIN_INCLUDES)
libbitcoin_wallet_a_SOURCES = \
  utiltest.cpp \
  utiltest.h \
  zcbenchmarks.cpp \
  zcbenchmarks.h \
  wallet/asyncrpcoperation_sendmany.cpp \
  wallet/crypter.cpp \
  wallet/db.cpp \
  wallet/rpcdump.cpp \
  wallet/rpcwallet.cpp \
  wallet/wallet.cpp \
  wallet/wallet_ismine.cpp \
  wallet/walletdb.cpp \
  crypto/scrypt.cpp \
  $(BITCOIN_CORE_H) \
  $(LIBZCASH_H)

EQUIHASH_TROMP_SOURCES = \
  pow/tromp/equi_miner.h \
  pow/tromp/equi.h \
  pow/tromp/osx_barrier.h

# crypto primitives library
crypto_libbitcoin_crypto_a_CPPFLAGS = $(BITCOIN_CONFIG_INCLUDES) -DEQUIHASH_TROMP_ATOMIC
crypto_libbitcoin_crypto_a_SOURCES = \
  crypto/common.h \
  crypto/equihash.cpp \
  crypto/equihash.h \
  crypto/equihash.tcc \
  crypto/hmac_sha256.cpp \
  crypto/hmac_sha256.h \
  crypto/hmac_sha512.cpp \
  crypto/hmac_sha512.h \
  crypto/ripemd160.cpp \
  crypto/ripemd160.h \
  crypto/sha1.cpp \
  crypto/sha1.h \
  crypto/sha256.cpp \
  crypto/sha256.h \
  crypto/sha512.cpp \
  crypto/sha512.h \
  ${EQUIHASH_TROMP_SOURCES}

# univalue JSON library
univalue_libbitcoin_univalue_a_SOURCES = \
  univalue/univalue.cpp \
  univalue/univalue.h \
  univalue/univalue_escapes.h \
  univalue/univalue_read.cpp \
  univalue/univalue_write.cpp

# common: shared between bitcoind, and bitcoin-qt and non-server tools
libbitcoin_common_a_CPPFLAGS = $(BITCOIN_INCLUDES)
libbitcoin_common_a_SOURCES = \
  amount.cpp \
  arith_uint256.cpp \
  base58.cpp \
  chainparams.cpp \
  cc/content.cpp \
  cc/link.cpp \
  cc/jsondb.cpp \
  coins.cpp \
  cc/domain.cpp \
  compressor.cpp \
  primitives/block.cpp \
  primitives/transaction.cpp \
  core_read.cpp \
  core_write.cpp \
  eccryptoverify.cpp \
  ecwrapper.cpp \
  hash.cpp \
  key.cpp \
  keystore.cpp \
  netbase.cpp \
  protocol.cpp \
  pubkey.cpp \
  scheduler.cpp \
  script/interpreter.cpp \
  script/script.cpp \
  script/script_error.cpp \
  script/sign.cpp \
  script/standard.cpp \
  $(BITCOIN_CORE_H) \
  $(LIBZCASH_H)

# util: shared between all executables.
# This library *must* be included to make sure that the glibc
# backward-compatibility objects and their sanity checks are linked.
libbitcoin_util_a_CPPFLAGS = $(BITCOIN_INCLUDES)
libbitcoin_util_a_SOURCES = \
  compat/strnlen.cpp \
  compat/glibc_sanity.cpp \
  compat/glibcxx_sanity.cpp \
  support/pagelocker.cpp \
  chainparamsbase.cpp \
  clientversion.cpp \
  random.cpp \
  rpcprotocol.cpp \
  support/cleanse.cpp \
  sync.cpp \
  uint256.cpp \
  util.cpp \
  utilstrencodings.cpp \
  utilmoneystr.cpp \
  utiltime.cpp \
  $(BITCOIN_CORE_H) \
  $(LIBZCASH_H)

if GLIBC_BACK_COMPAT
libbitcoin_util_a_SOURCES += compat/glibc_compat.cpp
libbitcoin_util_a_SOURCES += compat/glibcxx_compat.cpp
endif

# cli: shared between bitcoin-cli and bitcoin-qt
libbitcoin_cli_a_CPPFLAGS = $(BITCOIN_INCLUDES)
libbitcoin_cli_a_SOURCES = \
  rpcclient.cpp \
  $(BITCOIN_CORE_H) \
  $(LIBZCASH_H)

nodist_libbitcoin_util_a_SOURCES = $(srcdir)/obj/build.h
#

# bitcoind binary #
daemsd_SOURCES = bitcoind.cpp
daemsd_CPPFLAGS = $(BITCOIN_INCLUDES)
daemsd_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
daemsd_SOURCES += bitcoind-res.rc
endif

daemsd_LDADD = \
  $(LIBBITCOIN_SERVER) \
  $(LIBBITCOIN_COMMON) \
  $(LIBBITCOIN_UNIVALUE) \
  $(LIBBITCOIN_UTIL) \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBZCASH) \
  $(LIBLEVELDB) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)

if ENABLE_WALLET
daemsd_LDADD += libbitcoin_wallet.a
endif

daemsd_LDADD += \
  $(BOOST_LIBS) \
  $(BDB_LIBS) \
  $(SSL_LIBS) \
  $(CRYPTO_LIBS) \
  $(MINIUPNPC_LIBS) \
  $(LIBZCASH) \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBZCASH_LIBS)
#

# bitcoin-cli binary #
daems_cli_SOURCES = bitcoin-cli.cpp
daems_cli_CPPFLAGS = $(BITCOIN_INCLUDES)
daems_cli_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
daems_cli_SOURCES += bitcoin-cli-res.rc
endif

daems_cli_LDADD = \
  $(LIBBITCOIN_CLI) \
  $(LIBBITCOIN_UTIL) \
  $(BOOST_LIBS) \
  $(SSL_LIBS) \
  $(CRYPTO_LIBS) \
  $(LIBZCASH) \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBZCASH_LIBS)
#

# zcash-tx binary #
daems_tx_SOURCES = bitcoin-tx.cpp
daems_tx_CPPFLAGS = $(BITCOIN_INCLUDES)
daems_tx_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
daems_tx_SOURCES += bitcoin-tx-res.rc
endif

# FIXME: Is libzcash needed for zcash_tx?
daems_tx_LDADD = \
  $(LIBBITCOIN_UNIVALUE) \
  $(LIBBITCOIN_COMMON) \
  $(LIBBITCOIN_UTIL) \
  $(LIBSECP256K1) \
  $(LIBZCASH) \
  $(LIBBITCOIN_CRYPTO) \
  $(LIBZCASH_LIBS)

daems_tx_LDADD += $(BOOST_LIBS) $(CRYPTO_LIBS)
#

# zcash protocol primitives #
libzcash_a_SOURCES = \
  zcash/IncrementalMerkleTree.cpp \
  zcash/NoteEncryption.cpp \
  zcash/Address.cpp \
  zcash/JoinSplit.cpp \
  zcash/Proof.cpp \
  zcash/Note.cpp \
  zcash/prf.cpp \
  zcash/util.cpp \
  zcash/circuit/commitment.tcc \
  zcash/circuit/gadget.tcc \
  zcash/circuit/merkle.tcc \
  zcash/circuit/note.tcc \
  zcash/circuit/prfs.tcc \
  zcash/circuit/utils.tcc

libzcash_a_CPPFLAGS = -DMULTICORE -fopenmp -fPIC -DBINARY_OUTPUT -DCURVE_ALT_BN128 -DBOOST_SPIRIT_THREADSAFE -DHAVE_BUILD_INFO -D__STDC_FORMAT_MACROS $(HARDENED_CPPFLAGS) -pipe -O1 -g -Wstack-protector -fstack-protector-all -fPIE -fvisibility=hidden -DSTATIC $(BITCOIN_INCLUDES)

libzcash_a_CXXFLAGS = $(HARDENED_CXXFLAGS) -fwrapv -fno-strict-aliasing

libzcash_a_LDFLAGS = $(HARDENED_LDFLAGS)

libzcash_a_CPPFLAGS += -DMONTGOMERY_OUTPUT

# zcashconsensus library #
if BUILD_BITCOIN_LIBS
include_HEADERS = script/daemsconsensus.h
libdaemsconsensus_la_SOURCES = \
  primitives/transaction.cpp \
  crypto/hmac_sha512.cpp \
  crypto/scrypt.cpp \
  crypto/sha1.cpp \
  crypto/sha256.cpp \
  crypto/sha512.cpp \
  crypto/ripemd160.cpp \
  eccryptoverify.cpp \
  ecwrapper.cpp \
  hash.cpp \
  pubkey.cpp \
  script/script.cpp \
  script/interpreter.cpp \
  script/zcashconsensus.cpp \
  uint256.cpp \
  utilstrencodings.cpp

if GLIBC_BACK_COMPAT
  libdaemsconsensus_la_SOURCES += compat/glibc_compat.cpp
endif

libdaemsconsensus_la_LDFLAGS = -no-undefined $(RELDFLAGS)
libdaemsconsensus_la_LIBADD = $(CRYPTO_LIBS)
libdaemsconsensus_la_CPPFLAGS = $(CRYPTO_CFLAGS) -I$(builddir)/obj -DBUILD_BITCOIN_INTERNAL

endif
#

CLEANFILES = leveldb/libleveldb.a leveldb/libmemenv.a *.gcda *.gcno

DISTCLEANFILES = obj/build.h

EXTRA_DIST = leveldb

clean-local:
	-$(MAKE) -C leveldb clean
	-$(MAKE) -C secp256k1 clean
	rm -f leveldb/*/*.gcno leveldb/helpers/memenv/*.gcno
	-rm -f config.h

.rc.o:
	@test -f $(WINDRES)
	$(AM_V_GEN) $(WINDRES) -DWINDRES_PREPROC -i $< -o $@

.mm.o:
	$(AM_V_CXX) $(OBJCXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	  $(CPPFLAGS) $(AM_CXXFLAGS) $(QT_INCLUDES) $(CXXFLAGS)  -c -o $@ $<

check-symbols: $(bin_PROGRAMS)
if GLIBC_BACK_COMPAT
	@echo "Checking glibc back compat of [$(bin_PROGRAMS)]..."
	$(AM_V_at) READELF=$(READELF) CPPFILT=$(CPPFILT) $(top_srcdir)/contrib/devtools/symbol-check.py < $(bin_PROGRAMS)
endif

check-security: $(bin_PROGRAMS)
if HARDEN
	@echo "Checking binary security of [$(bin_PROGRAMS)]..."
	$(AM_V_at) READELF=$(READELF) OBJDUMP=$(OBJDUMP) $(top_srcdir)/contrib/devtools/security-check.py < $(bin_PROGRAMS)
endif

%.pb.cc %.pb.h: %.proto
	@test -f $(PROTOC)
	$(AM_V_GEN) $(PROTOC) --cpp_out=$(@D) --proto_path=$(abspath $(<D) $<)

if ENABLE_TESTS
include Makefile.test.include
include Makefile.gtest.include
endif

if ENABLE_QT
#include Makefile.qt.include
endif

if ENABLE_QT_TESTS
#include Makefile.qttest.include
endif

include Makefile.zcash.include
