#
# spec file for package yuca
#
# Copyright (c) 2018 SUSE LINUX GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://bugs.opensuse.org/
#


Name:           yuca
Version:        0.1.1+git.20180612
Release:        0
Summary:        A light-weight, in-memory, fast and simple to use search engine library
# FIXME: Select a correct license from https://github.com/openSUSE/spec-cleaner#spdx-licenses
License:        MIT
# FIXME: use correct group, see "https://en.opensuse.org/openSUSE:Package_group_guidelines"
Group:          Productivity/Databases/Tools
URL:            https://github.com/gubatron/yuca
Source:         %{name}-%{version}.tar.xz
Patch0:		001-install-target.patch
BuildRequires:  cmake
BuildRequires:  gcc-c++

%description
If your app can't handle or doesn't really need installing a full featured and heavy search engine like Lucene, 
nor you want to depend on a SQL database for indexing and doing simple search based strings you can use Yuca 
to index documents under any number of arbitrary keys which can be grouped under tags.

The shared library currently weighs ~170kb without any packing optimizations, we hope to reduce the size further in the near future.

%prep
%setup -q

%patch0 -p0

%build
%cmake
%make_build

%install
%cmake_install


%check
%ctest
./build/yuca_demo_shared
./build/yuca_tests

%post
%postun

%files
%license LICENSE
%doc README.md

%changelog
