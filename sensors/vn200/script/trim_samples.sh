rm -f $(ls -lh | sort -h -k 5 | awk '{print $9}' | head -100)
