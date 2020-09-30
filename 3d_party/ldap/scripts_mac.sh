/usr/libexec/slapd -d3 -f /Users/yakubov/Projects/asapo/3d_party/ldap/slapd.conf

ldapadd -x -D "ou=rgy,o=desy,c=de" -f record.ldif

ldapsearch -x -b ou=rgy,o=DESY,c=DE cn=a3p00-hosts