Name:	    smack
Summary:    SMACK library and utility executables
Version:    1.0
Release:    rc4slp2+s2
Group:      System/Libraries
License:    LGPL2.1
Source:    smack-%{version}.tar.gz
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


%build
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post utils
mkdir -p /smack
mkdir -p /opt/etc/smack
mkdir -p /opt/etc/smack/accesses.d
mkdir -p /opt/etc/smack/cipso.d

if [ ! -e /opt/etc/smack/accesses ] ; then
    touch /opt/etc/smack/accesses
fi
if [ ! -e /opt/etc/smack/cipso ] ; then
    touch /opt/etc/smack/cipso
fi

if [ ! -e /etc/smack ] ; then
    ln -s /opt/etc/smack /etc/smack
fi

mkdir -p /etc/rc.d/rc3.d
mkdir -p /etc/rc.d/rc4.d
mkdir -p /etc/rc.d/rc5.d
ln -s /etc/rc.d/init.d/smack-def /etc/rc.d/rc3.d/S07smack-def
ln -s /etc/rc.d/init.d/smack-def /etc/rc.d/rc4.d/S07smack-def
ln -s /etc/rc.d/init.d/smack-app /etc/rc.d/rc3.d/S08smack-app
ln -s /etc/rc.d/init.d/smack-app /etc/rc.d/rc5.d/S08smack-app

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
/usr/lib/libsmack.so.1
/usr/lib/libsmack.so.1.0.0

%files devel
%defattr(-,root,root,-)
/usr/lib/libsmack.so
/usr/include/sys/smack.h
/usr/lib/pkgconfig/libsmack.pc

%files utils
%defattr(-,root,root.-)
/bin/chsmack
/usr/bin/smackaccess
/sbin/smackcipso
/usr/bin/smackctl
/sbin/smackload
/usr/bin/smackd
/etc/rc.d/init.d/smack-def
/etc/rc.d/init.d/smack-app
