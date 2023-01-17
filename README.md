# libspp

This is a header-only C++ library for en/decoding [CCSDS SPP](https://public.ccsds.org/Pubs/133x0b2e1.pdf) packets.

Also contained are the following command-line tools:

* `sppinfo` - Displays the header contents of a CCSDS SPP packet stream from stdin.
* `sppfilter` - Filter CCSDS SPP packets that match any given selector from stdin to stdout.
* `spppack` - Pack bytes from stdin into a CCSDS SPP packet stream on stdout.
* `sppunpack` - Unpack a CCSDS SPP packet stream from stdin to stdout.

The available command line options are available from each program with the `-h` flag.

## Building

Dependencies:

* [libfec](https://github.com/quiet/libfec/)
* [libgetsetproxy](https://github.com/ssloxford/libgetsetproxy)
* [libseqiter](https://github.com/ssloxford/libseqiter)
* [libgiis](https://github.com/ssloxford/libgiis)

Building:

```
make
make install
```

# Further ideas

Detection of whether CADU is a "fill" CADU (as used in gov/nasa/gsfc/drl/rtstps/core/ccsds/CaduService.java) l202

# Thanks

Many thanks to [Jonathan Tanner](https://github.com/aDifferentJT) for his help in writing this library.
