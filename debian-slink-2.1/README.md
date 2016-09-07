The original ISOs *'debian-2.1r2-i386-binary-1.iso'* and *'debian-2.1r2-i386-binary-2.iso'* have been split into multiple files to enable uploading to github. Here are the split and merge commands.

#debian-2.1r2-i386-binary-1.iso -
```
split -d -e -b 95m debian-2.1r2-i386-binary-1.iso 'debian-2.1r2-i386-binary-1.iso.split-'

cat debian-2.1r2-i386-binary-1.iso.split-0* > debian-2.1r2-i386-binary-1.iso
```

#debian-2.1r2-i386-binary-2.iso -
```
split -d -e -b 95m debian-2.1r2-i386-binary-2.iso 'debian-2.1r2-i386-binary-2.iso.split-'

cat debian-2.1r2-i386-binary-2.iso.split-0* > debian-2.1r2-i386-binary-2.iso
```
