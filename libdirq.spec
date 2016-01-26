%define real_version %((cat %{_sourcedir}/VERSION || \
                        cat  %{_builddir}/VERSION || \
                        echo UNKNOWN) 2>/dev/null)

Summary:	C implementation of the simple directory queue algorithm
Name:		libdirq
Version:	%{real_version}
Release:	1%{?dist}
License:	ASL 2.0
Group:		System Environment/Libraries
Source0:	%{name}-%{version}.tgz
Source1:	VERSION
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root

%description
The goal of this library is to offer a "simple" queue system using the
underlying filesystem for storage, security and to prevent race conditions
via atomic operations. It focuses on simplicity, robustness and scalability.

Multiple concurrent readers and writers can interact with the same queue. 

Other implementations of the same algorithm exist so readers and writers can
be written in different programming languages.

%package devel
Summary:	Development files for %{name}
Requires:	%{name}%{?_isa} = %{version}-%{release}
Provides:	%{name}-static = %{version}-%{release}

%description devel
The %{name}-devel package contains header files, libraries and documentation
for developing programs using the %{name} library.

%prep
%setup -q
./configure --includedir=%{_includedir} --libdir=%{_libdir} --mandir=%{_mandir}

%build
make
make test

%install
make install INSTALLROOT=%{buildroot}

%clean
make clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/*.so.*

%files devel
%defattr(-,root,root,-)
%doc CHANGES DESIGN README doc/dirq.html src/dqt.c
%{_includedir}/*
%{_libdir}/*.a
%{_libdir}/*.so
%{_mandir}/*/*
