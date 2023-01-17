# libspp

This is a header-only C++ library for en/decoding [CCSDS SPP](https://public.ccsds.org/Pubs/133x0b2e1.pdf) packets.
It's

# Building

```
make
make install
```

# Ideas

Detection of whether CADU is a "fill" CADU (as used in gov/nasa/gsfc/drl/rtstps/core/ccsds/CaduService.java) l202

# Related work/documents

NASA en/decoders in RT-STPS and MODISL1DB_SPA
SpaceGroundAqua document
Other C++ library (decoding only, and not very nice)

# TODO

[ ] Factor out libgiis
[ ] sppinfo vs sppinfo2
[ ] Fix library names to spp
[ ] Fix remaining references to ccsds -> spp
