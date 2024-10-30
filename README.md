# dirq - C implementation of the simple directory queue algorithm

![Build Status](https://github.com/cern-mig/c-dirq/actions/workflows/test.yml/badge.svg)

> [!WARNING]
> This software is not maintained anymore.

## Description

The goal of this library is to offer a "simple" queue system using the
underlying filesystem for storage, security and to prevent race conditions
via atomic operations. It focuses on simplicity, robustness and scalability.

Multiple concurrent readers and writers can interact with the same queue.

Other implementations of the same algorithm exist so readers and writers can
be written in different programming languages:
  * C: https://github.com/cern-mig/c-dirq
  * Java: https://github.com/cern-mig/java-dirq
  * Perl: http://search.cpan.org/dist/Directory-Queue/
  * Python: https://github.com/cern-mig/python-dirq

## Installation

To see the available configuration options:
```
% ./configure --help
```

To configure, compile, test and install this library:
```
% ./configure
% make
% make test
% make install
```

## Documentation

After installation, you can find documentation for this library with the man
command:
```
% man 3 dirq
```

The man page is also available at http://cern-mig.github.io/c-dirq/

The Perl implementation is the reference one and contains extensive
documentation.

## Author

Lionel Cons - http://cern.ch/lionel.cons

## Copyright

Copyright (C) CERN 2012-2024
