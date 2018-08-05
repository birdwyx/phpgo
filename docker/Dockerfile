FROM centos:7

MAINTAINER michaelwei <4275556@qq.com>

ENV PHP_INSTALL_DIR /usr/local/php
ENV PHP_INI_DIR $PHP_INSTALL_DIR/etc
ENV MCRYPT_URL http://soft.vpser.net/web/libmcrypt/libmcrypt-2.5.8.tar.gz
ENV SWOOLE_VERSION v2.2.0

ENV PHP_CFLAGS="-fstack-protector-strong -fpic -fpie -O2"
ENV PHP_CPPFLAGS="$PHP_CFLAGS"
ENV PHP_LDFLAGS="-Wl,-O1 -Wl,--hash-style=both -pie"

ENV GPG_KEYS 0BD78B5F97500D450838F95DFE857D9A90D90EC1 6E4F6AB321FDC07F2C332E3AC2BF0BC433CFC8B3

ENV PHP_VERSION 7.2.4
ENV PHP_URL="http://cn2.php.net/get/php-$PHP_VERSION.tar.xz/from/this/mirror" PHP_ASC_URL="https://secure.php.net/get/php-$PHP_VERSION.tar.xz.asc/from/this/mirror"

ENV ICONV_VERSION 1.15
ENV ICONV_URL="https://ftp.gnu.org/pub/gnu/libiconv/libiconv-$ICONV_VERSION.tar.gz"

ENV PHP_SHA256="7916b1bd148ddfd46d7f8f9a517d4b09cd8a8ad9248734e7c8dd91ef17057a88" PHP_MD5=""

ENV RUNTIME_DEPS "libxml2 libxml2-devel zlib glib2 tar ncurses curl libcurl curl-devel libidn libcap diffutils xz ca-certificates libxslt libpng libjpeg freetype"

ENV RUNTIME_DEPS_PHP7 "libicu-devel libxslt libxslt-devel"

ENV BUILD_DEPS "make cmake gcc gcc-c++ flex bison file libtool libtool-libs autoconf kernel-devel patch wget crontabs gd gd-devel unzip bzip2 bzip2-devel libevent libevent-devel e2fsprogs e2fsprogs-devel krb5-devel openssl openssl-devel gettext gettext-devel gmp-devel net-tools psmisc libXpm-devel git-core c-ares-devel expat-devel"

ENV DEV_DEPS "git vim which"

COPY CentOS7-Base-163.repo php-fpm.conf /

RUN mv /etc/yum.repos.d /etc/yum.repos.d.backup4comex; \
	mkdir /etc/yum.repos.d; \
	mv /CentOS7-Base-163.repo /etc/yum.repos.d/CentOS-Base.repo; \
	yum clean all; \
	yum makecache; \
	rm -rf /etc/localtime; \
	ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime; \
	groupadd -g 1000 www; \
	useradd -s /sbin/nologin -u 1000 -g www www; 
	
RUN	yum update; \
	yum -y install $RUNTIME_DEPS $RUNTIME_DEPS_PHP7 $BUILD_DEPS $DEV_DEPS;

RUN	mkdir -p /usr/src; \
	cd /usr/src; \
	wget -O php.tar.xz "$PHP_URL"; \
	\
	if [ -n "$PHP_SHA256" ]; then \
		echo "$PHP_SHA256 *php.tar.xz" | sha256sum -c -; \
	fi; \
	if [ -n "$PHP_MD5" ]; then \
		echo "$PHP_MD5 *php.tar.xz" | md5sum -c -; \
	fi; \
	\
	if [ -n "$PHP_ASC_URL" ]; then \
		wget -O php.tar.xz.asc "$PHP_ASC_URL"; \
		export GNUPGHOME="$(mktemp -d)"; \
		for key in $GPG_KEYS; do \
			gpg --keyserver ha.pool.sks-keyservers.net --recv-keys "$key"; \
		done; \
		gpg --batch --verify php.tar.xz.asc php.tar.xz; \
		rm -rf "$GNUPGHOME"; \
	fi;
	
	#install libmcrypt
RUN	cd /usr/src; \
	wget --no-check-certificate -O libmcrypt.tar.gz "$MCRYPT_URL"; \
	mkdir -p /usr/src/libmcrypt; \
	tar -zxvf /usr/src/libmcrypt.tar.gz -C /usr/src/libmcrypt --strip-components=1; \
	cd /usr/src/libmcrypt; \
	./configure; \
	make && make install; \
	/sbin/ldconfig; \
	cd libltdl/; \
    ./configure --enable-ltdl-install; \
    make && make install; \
    ln -sf /usr/local/lib/libmcrypt.la /usr/lib/libmcrypt.la; \
    ln -sf /usr/local/lib/libmcrypt.so /usr/lib/libmcrypt.so; \
    ln -sf /usr/local/lib/libmcrypt.so.4 /usr/lib/libmcrypt.so.4; \
    ln -sf /usr/local/lib/libmcrypt.so.4.4.8 /usr/lib/libmcrypt.so.4.4.8; \
    ldconfig; \
	rm -rf /usr/src/libmcrypt.tar.gz; \
	rm -rf /usr/src/libmcrypt; \
	\
	mkdir -p /usr/src/php; \
	tar -Jxf /usr/src/php.tar.xz -C /usr/src/php --strip-components=1;
 
RUN	cd /usr/src; \
	wget -O libiconv.tar.gz "$ICONV_URL"; \
	ls -l /usr/src/; \
	mkdir -p /usr/src/libiconv; \
	tar -xvf /usr/src/libiconv.tar.gz -C /usr/src/libiconv --strip-components=1;
	
RUN	cd /usr/src/libiconv; \
	./configure --prefix=/usr/local; \
	make && make install; \
	echo /usr/local/lib >> /etc/ld.so.conf.d/iconv.conf; \
	ldconfig; \
	rm -rf /usr/src/libiconv;

RUN	cd /usr/src/php; \
	./configure --prefix=$PHP_INSTALL_DIR \
	--with-config-file-path=$PHP_INI_DIR \
	--with-config-file-scan-dir=$PHP_INSTALL_DIR/conf.d \
	--build=x86_64-linux-gnu \
	--with-fpm-user=www \
	--with-fpm-group=www \
	--with-mysqli=mysqlnd \
	--with-pdo-mysql=mysqlnd \
	--with-iconv-dir=/usr/local \
	--with-freetype-dir=/usr/local/freetype \
	--with-jpeg-dir \
	--with-png-dir \
	--with-zlib \
	--with-libxml-dir=/usr \
	--with-curl \
	--with-gd \
	--with-openssl \
	--with-mhash \
	--with-xmlrpc \
	--with-xsl \
	--with-gettext \
	--with-libzip \
	\
	--enable-fpm \
	--enable-xml \
	--enable-opcache \
	--enable-intl \
	--enable-calendar \
	--enable-bcmath \
	--enable-shmop \
	--enable-sysvsem \
	--enable-inline-optimization \
	--enable-mbregex \
	--enable-mbstring \
	--enable-ftp \
	--enable-pcntl \
	--enable-sockets \
	--enable-soap \
	--enable-maintainer-zts \
	\
	--disable-cgi \
	--disable-rpath \
	--disable-fileinfo; \
	touch ext/phar/phar.phar; \
	make ZEND_EXTRA_LIBS='-liconv'; \
	make install; \
	cp -r -f ./php.ini* $PHP_INI_DIR/; \
	cd /usr/src; \
	rm -rf /usr/src/php.tar.xz; \
	rm -rf /usr/src/php.tar.xz.asc; \
	#rm -rf /usr/src/php; \
	mkdir -p $PHP_INSTALL_DIR/{etc,conf.d};
	
	# add redis extension
RUN	if [ ! -d "/usr/src/redis" ]; then \
		mkdir -p /usr/src/redis; \
	fi; \
	cd /usr/src; \
	wget --no-check-certificate -O redis.tgz http://pecl.php.net/get/redis-3.1.3.tgz; \
	tar -zxvf /usr/src/redis.tgz -C /usr/src/redis --strip-components=1; \
	cd /usr/src/redis; \
	$PHP_INSTALL_DIR/bin/phpize; \
	./configure --with-php-config=$PHP_INSTALL_DIR/bin/php-config; \
	make && make install; \
	make clean; \
	rm -rf /usr/src/redis /usr/src/redis.tgz;
	
	# add swoole extension
RUN	if [ ! -d "/usr/src/swoole" ]; then \
		mkdir -p /usr/src/swoole; \
	fi; \
	cd /usr/src; \
	wget --no-check-certificate -O swoole.tar.gz https://github.com/swoole/swoole-src/archive/$SWOOLE_VERSION.tar.gz; \
	tar -zxvf /usr/src/swoole.tar.gz -C /usr/src/swoole --strip-components=1; \
	cd /usr/src/swoole; \
	$PHP_INSTALL_DIR/bin/phpize; \
	./configure --enable-openssl --with-php-config=$PHP_INSTALL_DIR/bin/php-config; \
	make && make install; \
	make clean; \
	rm -rf /usr/src/swoole /usr/src/swoole.tar.gz; \
	\
	yum clean all; \
	#yum -y autoremove $BUILD_DEPS; \
	rm -rf /var/cache/yum
	
RUN cp /php-fpm.conf $PHP_INI_DIR/; \
	cp -r -f /usr/src/php/php.ini-production $PHP_INI_DIR/php.ini; \
	cd $PHP_INI_DIR; \
	#mv redis.so swoole.so  $PHP_INSTALL_DIR/lib/php/extensions/no-debug-*; \
	sed -i '/extension=modulename.extension/a\extension=redis.so\n;extension=swoole.so' php.ini; \
	sed -i 's/post_max_size =.*/post_max_size = 100M/g' php.ini; \
	sed -i 's/upload_max_filesize =.*/upload_max_filesize = 100M/g' php.ini; \
	sed -i 's/;date.timezone =.*/date.timezone = PRC/g' php.ini; \
	sed -i 's/short_open_tag =.*/short_open_tag = On/g' php.ini; \
	sed -i 's/;cgi.fix_pathinfo=.*/cgi.fix_pathinfo=0/g' php.ini; \
	sed -i 's/max_execution_time =.*/max_execution_time = 300/g' php.ini; \
	\
	sed -i 's/;opcache.enable=.*/opcache.enable=1/g' php.ini; \
	sed -i 's/;opcache.memory_consumption=.*/opcache.memory_consumption=128/g' php.ini; \
	sed -i 's/;opcache.interned_strings_buffer=.*/opcache.interned_strings_buffer=8/g' php.ini; \
	sed -i 's/;opcache.max_accelerated_files=.*/opcache.max_accelerated_files=10000/g' php.ini; \
	sed -i 's/;opcache.revalidate_freq=.*/opcache.revalidate_freq=2/g' php.ini; \
	sed -i 's/;opcache.fast_shutdown=.*/opcache.fast_shutdown=1/g' php.ini; \
	\
	ln -sf /usr/local/php/bin/php /usr/bin/php; \
	ln -sf /usr/local/php/bin/phpize /usr/bin/phpize; \
	ln -sf /usr/local/php/bin/pear /usr/bin/pear; \
	ln -sf /usr/local/php/bin/pecl /usr/bin/pecl; \
	ln -sf /usr/local/php/sbin/php-fpm /usr/bin/php-fpm;

#COPY consul-template /bin
#RUN  chmod +x /bin/consul-template
WORKDIR $PHP_INSTALL_DIR/

EXPOSE 9000

CMD ["php-fpm","-y","/usr/local/php/etc/php-fpm.conf"]
