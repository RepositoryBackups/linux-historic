The original iso 'redhat-6.2-i386.iso' has been split into multiple files to enable uploading to github. Here are the split and merge commands.


split -d -e -b 95m redhat-6.2-i386.iso 'redhat-6.2-i386.iso.split-'

cat redhat-6.2-i386.iso.split-0* > redhat-6.2-i386.iso
