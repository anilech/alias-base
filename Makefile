# yum install 389-ds-base-devel
# 
PLUGINNAME = alias-base
#
CC = gcc
LD = gcc
CFLAGS = -fPIC -I /usr/include/nspr4 -Wall
LDFLAGS = -shared -z defs -L/usr/lib64/dirsrv -lslapd
OBJS = $(PLUGINNAME).o
# where to install plugin
LIBNAME = lib$(PLUGINNAME)-plugin.so
LIBPATH = /usr/lib64/dirsrv/plugins
# ldap connection parameters
SRVCON = -h localhost -p 389 -D "cn=Directory Manager" -y ~/.dmpwd
# bounce ldap server
SRVBNC = echo systemctl restart dirsrv@myserver
all: $(LIBNAME)
$(LIBNAME): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -c $<
clean:
	-rm -f $(OBJS) $(LIBNAME)
install: $(LIBNAME)
	[ "$$USER" == "root" ] || { echo "not root"; exit 1; }
	[ ! -f $(LIBPATH)/$(LIBNAME) ] && { cp $(LIBNAME) $(LIBPATH); chown root:root $(LIBPATH)/$(LIBNAME); chmod 755 $(LIBPATH)/$(LIBNAME); } || echo "$(LIBPATH)/$(LIBNAME) already exists!"
uninstall:
	[ "$$USER" == "root" ] || { echo "not root"; exit 1; }
	[ -f $(LIBPATH)/$(LIBNAME) ] && rm $(LIBPATH)/$(LIBNAME) || echo "$(LIBPATH)/$(LIBNAME) already removed"
replace: uninstall install
	$(SRVBNC)
register: $(PLUGINNAME).ldif
	[ "$$USER" == "root" ] || { echo "not root"; exit 1; }
	ldapadd $(LDAPPARS) -f $(PLUGINNAME).ldif
	$(SRVBNC)
unregister: $(PLUGINNAME).ldif
	[ "$$USER" == "root" ] || { echo "not root"; exit 1; }
	[ -f $(PLUGINNAME).ldif ] && ldapdelete $(LDAPPARS) "$$(awk '$$1=="dn:"{print $$2}' $(PLUGINNAME).ldif)" || echo "$(PLUGINNAME).ldif not found"
	$(SRVBNC)
enablepluginlogging:
	[ "$$USER" == "root" ] || { echo "not root"; exit 1; }
	echo -e "dn: cn=config\nchangetype: modify\nreplace: nsslapd-errorlog-level\nnsslapd-errorlog-level: 81920" |ldapmodify $(LDAPPARS) -x -o ldif_wrap=no
	$(SRVBNC)
disablepluginlogging:
	[ "$$USER" == "root" ] || { echo "not root"; exit 1; }
	echo -e "dn: cn=config\nchangetype: modify\nreplace: nsslapd-errorlog-level\nnsslapd-errorlog-level: 16384" |ldapmodify $(LDAPPARS) -x -o ldif_wrap=no
	$(SRVBNC)
