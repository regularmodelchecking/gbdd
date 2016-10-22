Summary: GBDD - A package for representing relations with BDDs
Name: gbdd
Version: 0.10
Release: 0
Copyright: GPL
Group: Applications
Source: gbdd-%{version}.tar.gz
URL: http://www.docs.uu.se/~marcusn/projects/rmc
Distribution: Department of Information Technology
Vendor: Department of Information Technology
Packager: Marcus Nilsson (marcusn@docs.uu.se)
Buildroot: /var/tmp/%{name}-root
%description
A package for representing relations with BDDs. Several BDD packages
can be connected, currently a binding for the BuDDy package is implemented.

%prep
%setup

%build
./configure --prefix=/usr --with-buddy
make

%install
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
/

