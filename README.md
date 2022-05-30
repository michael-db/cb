# cb (change base): convert a number from one base to another

`cb` is a filter program that does arbitrary radix conversions.

The filter is written in C (C99) for universality.
On a typical installation of Linux this should work:
```
make                        # create ./cb, run tests
sudo make install           # create /usr/bin/cb
cb                          # get a usage message

echo ff | cb 16 10          # 255
echo 19 | cb 10 2           # 1011
```
