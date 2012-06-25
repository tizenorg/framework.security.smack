Name:	    smack
Summary:    SMACK library and utility executables
Version:    1.0
Release:    rc4slp2+s2
Group:      System/Libraries
License:    LGPL2.1
Source:    smack-%{version}.tar.gz
Source1001: packaging/smack.manifest
Patch0:     0001-systemd-path-change.patch
Patch1:     0002-libsmack-fix-unit-file-ordering.patch

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires: cmake

%description
SMACK library and utility executables


%package devel
Summary:    SMACK library header
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%package utils
Summary:	SMACK utility executables
Group:		System/Executables
Requires:   %{name} = %{version}-%{release}

%description devel
SMACK library  (developement files)

%description utils
SMACK utility executables

%prep
%setup -q
%patch0 -p1
%patch1 -p1

%build
cp %{SOURCE1001} .
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p ${RPM_BUILD_ROOT}%{_sysconfdir}/smack
mkdir -p ${RPM_BUILD_ROOT}%{_sysconfdir}/smack/accesses.d
mkdir -p ${RPM_BUILD_ROOT}%{_sysconfdir}/smack/cipso.d
mkdir -p ${RPM_BUILD_ROOT}%{_libdir}/systemd/system/local-fs.target.wants/
mkdir -p ${RPM_BUILD_ROOT}%{_libdir}/systemd/system/basic.target.wants/
mkdir -p ${RPM_BUILD_ROOT}/%{_sysconfdir}/rc.d/rc3.d
mkdir -p ${RPM_BUILD_ROOT}/%{_sysconfdir}/rc.d/rc4.d
mkdir -p ${RPM_BUILD_ROOT}/%{_sysconfdir}/rc.d/rc5.d

install -m0644 init/smack.mount ${RPM_BUILD_ROOT}%{_libdir}/systemd/system/
install -m0644 init/smack.service ${RPM_BUILD_ROOT}%{_libdir}/systemd/system/
ln -sf %{_libdir}/systemd/system/smack.mount ${RPM_BUILD_ROOT}%{_libdir}/systemd/system/local-fs.target.wants/smack.mount
ln -sf %{_libdir}/systemd/system/smack.service  ${RPM_BUILD_ROOT}%{_libdir}/systemd/system/basic.target.wants/smack.service

ln -s %{_sysconfdir}/rc.d/init.d/smack-def ${RPM_BUILD_ROOT}/%{_sysconfdir}/rc.d/rc3.d/S07smack-def
ln -s %{_sysconfdir}/rc.d/init.d/smack-def ${RPM_BUILD_ROOT}/%{_sysconfdir}/rc.d/rc4.d/S07smack-def
ln -s %{_sysconfdir}/rc.d/init.d/smack-app ${RPM_BUILD_ROOT}/%{_sysconfdir}/rc.d/rc3.d/S08smack-app
ln -s %{_sysconfdir}/rc.d/init.d/smack-app ${RPM_BUILD_ROOT}/%{_sysconfdir}/rc.d/rc5.d/S08smack-app

%post utils
touch %{_sysconfdir}/smack/accesses
touch %{_sysconfdir}/smack/cipso

# Base package - Shutdown service
%postun -p /sbin/ldconfig
%post -p /sbin/ldconfig

%files
%manifest smack.manifest
%defattr(-,root,root,-)
%{_libdir}/libsmack.so.1
%{_libdir}/libsmack.so.1.0.0

%files devel
%manifest smack.manifest
%defattr(-,root,root,-)
%{_libdir}/libsmack.so
%{_includedir}/sys/smack.h
%{_libdir}/pkgconfig/libsmack.pc

%files utils
%manifest smack.manifest
%defattr(-,root,root.-)
%defattr(-,root,root,-)
%{_sysconfdir}/rc.d/init.d/smack-app
%{_sysconfdir}/rc.d/init.d/smack-def
%{_sysconfdir}/rc.d/rc3.d/S07smack-def
%{_sysconfdir}/rc.d/rc4.d/S07smack-def
%{_sysconfdir}/rc.d/rc3.d/S08smack-app
%{_sysconfdir}/rc.d/rc5.d/S08smack-app
%dir %{_sysconfdir}/smack/accesses.d
%dir %{_sysconfdir}/smack/cipso.d
%{_libdir}/systemd/system/smack.mount
%{_libdir}/systemd/system/local-fs.target.wants/smack.mount
%{_libdir}/systemd/system/smack.service
%{_libdir}/systemd/system/basic.target.wants/smack.service
/bin/chsmack
/sbin/smackcipso
/sbin/smackload
%{_bindir}/smackctl
%{_bindir}/smackaccess
%{_bindir}/smackd

%changelog
