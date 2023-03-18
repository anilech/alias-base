![kdpv](https://anile.ch/assets/flachau.jpg)
# alias-base

The dereference of aliases is unfortunately [not supported](#Links) by the [389ds ldap server](https://www.port389.org/). Therefore here is a small [plugin](https://github.com/anilech/alias-base) which resolves aliases during **base** search. Subtree and onelevel searches are not supported.

## Compiling
```
yum install 389-ds-base-devel
make
```
  
## Installing
```
cp libalias-base-plugin.so /usr/lib64/dirsrv/plugins
chown root:root /usr/lib64/dirsrv/plugins/libalias-base-plugin.so
chmod 755 /usr/lib64/dirsrv/plugins/libalias-base-plugin.so
ldapadd -H ldap://ldapserver -D "cn=Directory Manager" -W -f alias-base.ldif
systemctl restart dirsrv@ldapserver
```

## Removing
```
ldapdelete -H ldap://ldapserver -D "cn=Directory Manager" -W "cn=alias-base,cn=plugins,cn=config"
systemctl restart dirsrv@ldapserver
rm /usr/lib64/dirsrv/plugins/libalias-base-plugin.so
```

## Why
Consider you have the Oracle database MYDB, and you keep your tnsnames in the LDAP:

```
dn: cn=MYDB,cn=OracleContext,dc=world
objectClass: top
objectClass: orclNetService
cn: MYDB
orclNetDescString: (DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)(HOST=DBHOST)(PORT=1521))(CONNECT_DATA=(SERVICE_NAME=MYDB)))
```

Now you have migrated the data from the other db MYOLDDB into this MYDB. On the clients side the tnsname MYOLDDB is still configured. To enable clients to connect to the new DB without changing their settings, you can create an alias like this:

```
dn: cn=MYOLDDB,cn=OracleContext,dc=world
objectClass: top
objectClass: alias
objectClass: orclNetServiceAlias
cn: MYOLDDB
aliasedObjectName: cn=MYDB,cn=OracleContext,dc=world
```

## Example
Here is what happening with and without plugin:

<table>
<tr><th>without plugin<th>with plugin
<tr><td colspan=2>

`$ ldapsearch -a find -s base -x -LLL -H ldap://ldapserver -b "cn=MYDB,cn=OracleContext,dc=world" orclNetDescString`

<tr><td>

```
dn: cn=MYDB,cn=OracleContext,dc=world
orclNetDescString: (DESCRIPTION=...
```

<td>

```
dn: cn=MYDB,cn=OracleContext,dc=world
orclNetDescString: (DESCRIPTION=...
```

<tr><td colspan=2>

`ldapsearch -a find -s base -x -LLL -H ldap://ldapserver -b "cn=MYOLDDB,cn=OracleContext,dc=world" orclNetDescString`

<tr><td>

```
dn: cn=MYOLDDB,cn=OracleContext,dc=world
```
> :warning: orclNetDescString not returned
<td>

```
dn: cn=MYDB,cn=OracleContext,dc=world
orclNetDescString: (DESCRIPTION=...
```
> :white_check_mark: orclNetDescString is here and dn is MYDB
<tr><td colspan=2>

`tnsping MYDB`

<tr><td>

```
Used parameter files:
c:\...\OraCli193_64\network\admin\sqlnet.ora

Used LDAP adapter to resolve the alias
Attempting to contact (DESCRIPTION=...
OK (20 msec)
```

<td>

```
Used parameter files:
c:\...\OraCli193_64\network\admin\sqlnet.ora

Used LDAP adapter to resolve the alias
Attempting to contact (DESCRIPTION=...
OK (20 msec)
```
<tr><td colspan=2>

`tnsping MYOLDDB`

<tr><td>

```
Used parameter files:
c:\...\OraCli193_64\network\admin\sqlnet.ora

TNS-03505: Failed to resolve name
```
> :warning: The client throws TNS-03505
<td>

```
Used parameter files:
c:\...\OraCli193_64\network\admin\sqlnet.ora

Used LDAP adapter to resolve the alias
Attempting to contact (DESCRIPTION=...
OK (20 msec)
```

</table>

## Links
* 389ds alias issue 389ds/389-ds-base#152
* Plugin docu https://access.redhat.com/sites/default/files/attachments/red_hat_directory_server-10-plug-in_guide-en-us.pdf
* Alias RFC https://www.rfc-editor.org/rfc/rfc4512#section-2.6
