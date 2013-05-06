Name:       smack
Version:    1.0slp2+s11
Release:    1
Summary:    Package to interact with Smack
Group:      System/Kernel
License:    LGPLv2
URL:        https://github.com/organizations/smack-team/smack
Source0:    smack-%{version}.tar.gz
BuildRoot:  %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: automake autoconf libtool

%description
Library allows applications to work with Smack

%package devel
Summary:    Developmnent headers and libs for libsmack
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Standard header files for use when developing Smack enabled applications

%package utils
Summary:    Selection of tools for developers working with Smack
Group:      System/Kernel
Requires:   %{name} = %{version}-%{release}

%description utils
Tools provided to load and unload rules from the kernel and query the policy

%prep
%setup -q
autoreconf --install --symlink

%build
%configure --with-systemdsystemunitdir=%{_libdir}/systemd/system
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}
install -d %{buildroot}/smack
install -d %{buildroot}/etc
install -D -d %{buildroot}/opt/etc/smack/accesses.d
install -D -d %{buildroot}/opt/etc/smack/cipso.d
install -D -d %{buildroot}/etc/rc.d/rc3.d/
install -D -d %{buildroot}/etc/rc.d/rc4.d/
install -D init/smack.rc %{buildroot}/etc/init.d/smack-utils
#ln -sf /opt/etc/smack %{buildroot}/etc/
ln -sf /etc/init.d/smack-utils %{buildroot}/etc/rc.d/rc3.d/S01smack
ln -sf /etc/init.d/smack-utils %{buildroot}/etc/rc.d/rc4.d/S01smack
install -D -d %{buildroot}%{_libdir}/systemd/system/local-fs.target.wants
install -D -d %{buildroot}%{_libdir}/systemd/system/basic.target.wants
ln -sf ../%{name}.mount %{buildroot}%{_libdir}/systemd/system/local-fs.target.wants/
ln -sf ../%{name}.service %{buildroot}%{_libdir}/systemd/system/basic.target.wants/
rm -rf %{buildroot}/%{_docdir}

%clean
rm -rf %{buildroot}

%post utils
if [ -d /etc/smack ]; then
	cp -r /etc/smack /opt/etc/
	rm -rf /etc/smack
fi
ln -sf /opt/etc/smack /etc/

%postun -p /sbin/ldconfig

%files
%defattr(644,root,root,755)
%{_libdir}/libsmack.so.*

%files devel
%defattr(644,root,root,755)
%{_includedir}/*
%{_libdir}/libsmack.so
%{_libdir}/libsmack.la
%{_libdir}/pkgconfig/*
%{_mandir}/man3/*

%files utils
%defattr(644,root,root,755)
%attr(755,root,root) %{_bindir}/*
%attr(755,root,root) /etc/init.d/smack-utils
#/etc/smack
/etc/rc.d/*
%{_libdir}/systemd/system/%{name}.mount
%{_libdir}/systemd/system/local-fs.target.wants/%{name}.mount
%{_libdir}/systemd/system/%{name}.service
%{_libdir}/systemd/system/basic.target.wants/%{name}.service
/opt/etc/*
/smack/
%{_mandir}/man1/*
%{_mandir}/man8/*

%changelog
* Wed Apr 24 2013 Rafal Krypa <r.krypa@samsung.com> - 1.0slp2+s11
- libsmack: check label length in smack_revoke_subject().
- Merge changes from upstream repository:
  - libsmack: fallback to short labels.
  - Declare smack_mnt as non-static in init.c.
  - Removed dso.h.
  - smack.service: provide [Install] section in systemd unit file.
  - smack.mount: "WantedBy" is illegal in [Unit] context.
  - Move cipso_free,cipso_new,cipso_apply from utils/common.c to libsmack/libsmack.c.
  - Add support for smackfs directory: /sys/fs/smackfs/
  - smackcipso can't set CIPSO correctly (fixes bug TDIS-3891)
  - Run AM_PROG_AR to fix build with newer automake.
  - disable services for new systemd versions

* Thu Feb 07 2013 Rafal Krypa <r.krypa@samsung.com> - 1.0slp2+s9
- Polish init script.
- execute init script between local-fs.target and basic.target.
- libsmack: fix access type parsing.
- libsmack: fix label removal.
- Don't fail when removing label from file, that doesn't have it.

* Wed Dec 10 2012 Jacek Migacz <j.migacz@samsung.com> - 1.0slp2+s8
- Add systemd support scripts.

* Mon Nov 26 2012 Kidong Kim <kd0228.kim@samsung.com> - 1.0slp2+s7
- fix initialization script order : S07 -> S01

* Mon Oct 29 2012 Tomasz Swierczek <t.swieczek@samsung.com> - 1.0slp2+s6
- No changes, re-release to proper OBS project on tizendev.

* Thu Oct 25 2012 Tomasz Swierczek <t.swieczek@samsung.com> - 1.0slp2+s6
- No changes, re-release to proper OBS project on tizendev.

* Mon Sep 17 2012 Rafal Krypa <r.krypa@samsung.com> - 1.0slp2+s6
- Modified typo access.d --> accesses.d
- packaging: fix location of symlinks to smack-utils init script.
- Merge with upstream.

* Thu Aug  1 2012 Rafal Krypa <r.krypa@samsung.com> - 1.0slp2+s5
- Rebuild, no source changes.

* Thu Jul 30 2012 Rafal Krypa <r.krypa@samsung.com> - 1.0slp2+s4
- Rebuild, no source changes.

* Thu Jul 19 2012 Rafal Krypa <r.krypa@samsung.com> - 1.0slp2+s3
- Rebuild, change versioning schema.

* Wed Jul 11 2012 Rafal Krypa <r.krypa@samsung.com> - 1.0-slp2+s2
- Release with my source patches after review with the upstream maintainer.

* Wed May  9 2012 Rafal Krypa <r.krypa@samsung.com> - 1.0-slp2+s1
- Initial spec file
