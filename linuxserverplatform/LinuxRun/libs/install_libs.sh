cp ./libcrypto.so.10 /usr/local/lib/
cp ./libcurl.so.4 /usr/local/lib/
cp ./libevent-2.2.so.1 /usr/local/lib/
cp ./libevent_pthreads-2.2.so.1 /usr/local/lib/
cp ./libhiredis.a /usr/local/lib/
cp ./libjson.so /usr/local/lib/
cp ./libmysqlclient.so.20 /usr/local/lib/
cp ./libssh2.so.1 /usr/local/lib/
cp ./libssl.so.10 /usr/local/lib/

echo "copy success !!!"

ln -is /usr/local/lib/libcurl.so.4 /usr/local/lib/libcurl.so
ln -is /usr/local/lib/libevent-2.2.so.1 /usr/local/lib/libevent.so
ln -is /usr/local/lib/libevent_pthreads-2.2.so.1 /usr/local/lib/libevent_pthreads.so
ln -is /usr/local/lib/libmysqlclient.so.20 /usr/local/lib/libmysqlclient.so

echo "link success !!!"
