install libdb 6.2:
ftp://ftp7.freebsd.org/sites/www.linuxfromscratch.org/blfs/view/systemd/server/db.html
install libsnark:
https://github.com/scipr-lab/libsnark
add " CPPFLAGS="-I/installpath/include -O2" LDFLAGS="-L/installpath/lib" to ./configure command
