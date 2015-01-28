#sbs-git:slp/pkgs/d/device-manager-plugin-exynos device-manager-plugin-exynos 0.0.1 5bf2e95e0bb15c43ff928f7375e1978b0accb0f8
Name:       device-manager-plugin-exynos
Summary:    Device manager plugin exynos
Version: 0.0.26
Release:    0
Group:      TO_BE/FILLED_IN
License:    TO_BE/FILLED_IN
Source0:    %{name}-%{version}.tar.gz
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  cmake
BuildRequires:  pkgconfig(devman_plugin)
BuildRequires:  pkgconfig(hwcommon)

%description
Device manager plugin exynos


%prep
%setup -q

%build
cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp LICENSE %{buildroot}/usr/share/license/%{name}
%make_install


%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%manifest device-manager-plugin-exynos.manifest
/usr/lib/libslp_devman_plugin.so
%{_libdir}/hw/*.so
/usr/share/license/%{name}
