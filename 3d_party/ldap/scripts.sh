/opt/asapo/ldap/slapd -f /opt/asapo/ldap/ldap.conf

ldapadd -x -D "ou=rgy,o=desy,c=de" -f record.ldif -h localhost

ldapsearch -x -b ou=rgy,o=DESY,c=DE cn=a3p00-hosts -h localhost