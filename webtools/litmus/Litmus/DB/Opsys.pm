# -*- Mode: perl; indent-tabs-mode: nil -*-
#
# The contents of this file are subject to the Mozilla Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Original Code is Litmus.
#
# The Initial Developer of the Original Code is Netscape Communications
# Corporation. Portions created by Netscape are
# Copyright (C) 1998 Netscape Communications Corporation. All
# Rights Reserved.
#
# Contributor(s): Zach Lipton <zach@zachlipton.com>

package Litmus::DB::Opsys;

use strict;
use base 'Litmus::DBI';

Litmus::DB::Opsys->table('opsyses');

Litmus::DB::Opsys->columns(All => qw/opsysid platform name detect_regexp/);

Litmus::DB::Opsys->has_many(testresults => "Litmus::DB::Testresult");
Litmus::DB::Opsys->has_a(platform => "Litmus::DB::Platform");

1;