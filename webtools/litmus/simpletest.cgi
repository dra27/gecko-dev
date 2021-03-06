#!/usr/bin/perl -w
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

use strict;

use Litmus;
use Litmus::Error;
use Litmus::DB::Test;
use Litmus::UserAgentDetect;
use Litmus::SysConfig;
use Litmus::Auth;

use CGI;
use Time::Piece::MySQL;

my $c = new CGI; 

# how old of a build do we want to allow? default is 10 days
my $maxbuildage = 10; 
# what branch do we accept? default is Deer Park
my $branch = Litmus::DB::Branch->retrieve(1);

showTest();

sub showTest {
    print $c->header();
    
    # if they don't have a buildid or it's old, then just don't display 
    # any tests:
    my $ua = Litmus::UserAgentDetect->new();
    my $time = localtime;
    $time->date_separator("");
    my $curbuildtime = $time->ymd;
    my $prod = Litmus::DB::Product->search(name => "Firefox")->next();
    my $branch = Litmus::DB::Branch->search(product => $prod, name => "Trunk")->next();
    my @detectbranch = $ua->branch($prod);
    if (! $ua->buildid() || ! $detectbranch[0] || 
            $detectbranch[0]->branchid() != $branch->branchid() || 
            $curbuildtime - $ua->buildid() > $maxbuildage) {
        Litmus->template()->process("simpletest/simpletest.html.tmpl") || 
            internalError(Litmus->template()->error());
        exit;
    }
    
    my $pid = $prod->productid();
    # get a random test to display:
    Litmus::DB::Test->set_sql(random_test => qq {
        SELECT __ESSENTIAL__
        FROM __TABLE__, products,testgroups,subgroups
        WHERE
            products.productid=? AND 
            communityenabled = 1 AND
            products.productid=testgroups.product AND 
            subgroups.testgroup=testgroups.testgroupid AND 
            tests.subgroup=subgroups.subgroupid 
        ORDER BY RAND()    
        LIMIT 1
    });

    my $test = Litmus::DB::Test->search_random_test($pid)->next();
    
    my @results = Litmus::DB::Result->retrieve_all();
    
    my $vars = {
        test => $test,
        results => \@results,
    };
    
    Litmus->template()->process("simpletest/simpletest.html.tmpl", $vars) || 
        internalError(Litmus->template()->error());
}
