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

# Handle detection of system information from the UA string

package Litmus::UserAgentDetect;

use strict;

require Exporter;
use Litmus;
use Litmus::DB::Platform;
use Litmus::DB::Opsys;
use Litmus::DB::Branch;

our @ISA = qw(Exporter);
our @EXPORT = qw(detectBuildID);

# define some SQL queries we will use:
Litmus::DB::Platform->set_sql(detectplatform => qq{
                                                  SELECT __ESSENTIAL__
                                                  FROM __TABLE__
                                                  WHERE 
                                                      ? LIKE detect_regexp AND 
                                                      product LIKE ?
                                                });
Litmus::DB::Branch->set_sql(detectbranch => qq{
                                                SELECT __ESSENTIAL__
                                                FROM __TABLE__
                                                WHERE 
                                                    ? LIKE detect_regexp AND 
                                                    product LIKE ?
                                            });                                                

# constructor. Optionally you can pass a UA string 
# and it will be used. Otherwise the default is the 
# current useragent.
sub new {
    my $self = {};
    my $class = shift;
    my $ua = shift;
    
    bless($self);
    $self->{ua} = $main::ENV{"HTTP_USER_AGENT"};
    if ($ua) { $self->{ua} = $ua }
    return $self;
}

# default stringification is to return the ua:
use overload 
    '""' => \&ua;
    

sub ua {
    my $self = shift;
    
    # we pad the UA with a space since some of our regexp matches expect 
    # to match things at the end of the string. This is quite possibly 
    # a bug. 
    return $self->{ua}." ";
}

sub buildid {
    my $self = shift;
    my $ua = $self->{ua};
    
    # mozilla products only
    unless ($ua =~ /Mozilla\/5\.0/) {
        return undef;
    }
    $ua =~ /(200\d*)/;
    return $1; 
}

sub platform {
    my $self = shift;
    my $product = shift; # optionally, just lookup for one product
    
    if (! $product) { $product = '%' }
    
    my @platforms = Litmus::DB::Platform->search_detectplatform($self->ua, $product);
    return @platforms;
}

sub branch {
    my $self = shift;
    my $product = shift; # optionally, just lookup for one branch
    
    if (! $product) { $product = '%' }
    
    my @branches = Litmus::DB::Branch->search_detectbranch($self->ua, $product);
    return @branches;
}

# from the legacy API before we had an OO interface:
sub detectBuildId() {
    my $self = Litmus::UserAgentDetect->new($main::ENV{"HTTP_USER_AGENT"});
    
    return $self->buildid();
}

1;