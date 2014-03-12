#!/bin/sh
#
#
#         Name: CMSFSTAB SH (shell script)
#               Adds TAB characters required by many 'make' variants.
#       Author: Rick Troth, BMC Software, Inc., Houston, Texas, USA
#         Date: 2000-Nov-03 (Fri)
#
#
 
#
# insert required TABs into the distribution makefile ...
#echo " " | awk '{printf "s/^        /\t/"}' > $$.sed
SED=`echo " " | awk '{printf "s/^        /\t/"}'`
#sed -f $$.sed
sed "$SED"
#rm $$.sed
 
exit
 
